"""A simple documentation server."""

import optparse
import tornado.web
import tornado.ioloop

class MainHandler(tornado.web.RequestHandler):
    """We're supposed to be able to just use StaticFileHandler for this, but it
    doesn't work. Weird.
    """

    def get(self):
        self.set_header('Cache-Control', 'max-age=0')
        with open('docs/jsdoc.html') as f:
            self.write(f.read())

application = tornado.web.Application([
        ('/', MainHandler)], static_path='static/')

if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('-p', '--port', type='int', default=8888, help='The port to listen on')
    opts, args = parser.parse_args()

    application.listen(opts.port)
    try:
        tornado.ioloop.IOLoop.instance().start()
    except KeyboardInterrupt:
        pass
