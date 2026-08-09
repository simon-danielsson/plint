#!/usr/bin/env python3

"""

Source:
https://aosabook.org/en/500L/a-simple-web-server.html

"""

from http import server

class RequestHandler(server.BaseHTTPRequestHandler):
    """Handle HTTP requests by returning a fixed 'page'."""

    Page = """\
            <html>
<body>
<p>Hello, web!</p>
</body>
</html>
"""

    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.send_header("Content-Length", str(len(self.Page)))
        self.end_headers()
        self.wfile.write(self.Page.encode())

# ----------------------------------------------------------------------

if __name__ == "__main__":
    serverAddress = ("", 8080)
    server = server.HTTPServer(serverAddress, RequestHandler)
    server.serve_forever()
