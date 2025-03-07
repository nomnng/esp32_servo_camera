import asyncio
import tornado
import tornado.websocket
import tornado.tcpserver
from tornado.iostream import StreamClosedError

TCP_PORT = 7788

class DataBridge:
    tcp_stream = None
    ws_clients = []

    @classmethod
    def set_tcp_stream(cls, stream):
        cls.tcp_stream = stream

    @classmethod
    def add_ws_client(cls, client):
        cls.ws_clients.append(client)

    @classmethod
    def remove_ws_client(cls, client):
        cls.ws_clients.remove(client)

    @classmethod
    def forward_to_tcp(cls, data):
        if cls.tcp_stream:
            cls.tcp_stream.write(data)

    @classmethod
    def forward_to_ws_clients(cls, data):
        for client in cls.ws_clients:
            client.write_message(data, binary=True)


class TCPServer(tornado.tcpserver.TCPServer):
    async def handle_stream(self, stream, address):
        print("Tcp stream opened")
        DataBridge.set_tcp_stream(stream)

        while True:
            try:
                image_size_bytes = await stream.read_bytes(4, False)
                image_size = int.from_bytes(image_size_bytes, "little")
                image_bytes = await stream.read_bytes(image_size, False)
                DataBridge.forward_to_ws_clients(image_bytes)
            except StreamClosedError:
                print("Tcp stream closed")
                break


class WSHandler(tornado.websocket.WebSocketHandler):
    def open(self):
        print("WebSocket opened")
        DataBridge.add_ws_client(self)

    def on_message(self, message):
        if type(message) != bytes:
            message = bytes(message, "utf8")
        DataBridge.forward_to_tcp(message)

    def on_close(self):
        print("WebSocket closed")
        DataBridge.remove_ws_client(self)


async def main():
    handlers = [
        (r"/ws", WSHandler),
        (r"/(.*)", tornado.web.StaticFileHandler, {"path": "./static", "default_filename": "index.html"}),
    ]
    web_app = tornado.web.Application(
        handlers,
        debug=True,
        static_handler_args=dict(default_filename="index.html")
    )
    web_app.listen(80)

    tcp_server = TCPServer()
    tcp_server.listen(TCP_PORT)

    await asyncio.Event().wait()


if __name__ == "__main__":
    asyncio.run(main())
