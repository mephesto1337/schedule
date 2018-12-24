#! /usr/bin/env python3

import argparse
import logging
import random
import socket
import socketserver
import sys
import time

logging.basicConfig(
    stream=sys.stderr,
    level=logging.INFO,
    format='[%(asctime)s] [%(levelname)-7s] %(message)s'
)
logger = logging.getLogger(__name__)
args = None


class RequestHandler(socketserver.BaseRequestHandler):
    def setup(self):
        logger.info('New client from %s:%d', *self.client_address)

    def handle(self):
        for _ in range(5):
            n = random.randint(1, 1024)
            self.request.send(bytes(n))
            sleep_duration = random.random() / 2
            logger.info(
                'Will sleep for %.02f seconds for %s:%d',
                sleep_duration, *self.client_address
            )
            time.sleep(sleep_duration)
        logger.info('Shutdown %s:%d', *self.client_address)
        self.request.shutdown(socket.SHUT_RDWR)


class Server(socketserver.ThreadingTCPServer):
    allow_reuse_address = True

    def __init__(self, port):
        socketserver.ThreadingTCPServer.__init__(
            self, ('0.0.0.0', port), RequestHandler, True
        )


def main():
    def parse_args():
        global args
        parser = argparse.ArgumentParser(
            formatter_class=argparse.ArgumentDefaultsHelpFormatter
        )
        parser.add_argument(
            '-p', '--port', dest='port', type=int, default=1337, metavar='PORT',
            help='TCP port to listen on'
        )
        args = parser.parse_args()
        return args

    server = None
    try:
        logger.debug('args = %r', parse_args())
        server = Server(args.port)
        logger.info('Listenning on %d', args.port)
        server.serve_forever()
        return 0
    except KeyboardInterrupt:
        server.shutdown()
        return 0
    except Exception as e:
        te = type(e)
        show_backtrace = bool(logger.getEffectiveLevel() <= logging.DEBUG)
        logger.error(
            'Caught %s.%s : %s',
            te.__module__, te.__name__, str(e),
            exc_info=show_backtrace
        )
        return 1


if __name__ == '__main__':
    sys.exit(main())
