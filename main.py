import asyncio
import threading
import serial
from bayes_opt import BayesianOptimization
from bayes_opt.util import UtilityFunction

try:
    import json
    import tornado.ioloop
    import tornado.httpserver
    from tornado.web import RequestHandler
    import requests
except ImportError:
    raise ImportError(
        "In order to run this example you must have the libraries: " +
        "`tornado` and `requests` installed."
    )

# hyper parameter
Number_of_iter = 30

# for bayesian
Kappa = 3
Xi = 1

serial_port = 'COM4'

ser = serial.Serial(serial_port, 9600, timeout=1)  # 根据实际串口号和波特率设置


def black_box_function(P, I, D):
    """Function with unknown internals we wish to maximize.

    This is just serving as an example, however, for all intents and
    purposes think of the internals of this function, i.e.: the process
    which generates its outputs values, as unknown.
    """

    pid_bytes = bytes([P, I, D])
    try:
        # Send PID bytes to serial port
        ser.write(pid_bytes)
        print(f"Sent PID values: P={P}, I={I}, D={D} to serial port {serial_port}")
    except serial.SerialException as e:
        print(f"Error writing to serial port: {e}")

    answer = 0

    try:
        while True:
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting)
                print("接收到的数据:", data.decode('utf-8'))  # 解码为字符串并打印
                answer = data.decode('utf-8')
    except serial.SerialException as e:
        print("串口读取错误:", e)

    return answer


class BayesianOptimizationHandler(RequestHandler):
    """Basic functionality for NLP handlers."""
    PID_para = {"D": (2, 15), "I": (0, 100), "P": (80, 100)}
    _bo = BayesianOptimization(
        f=black_box_function,
        pbounds=PID_para
    )
    _uf = UtilityFunction(kind="ucb", kappa=Kappa, xi=Xi)

    def post(self):
        """Deal with incoming requests."""
        body = tornado.escape.json_decode(self.request.body)

        try:
            self._bo.register(
                params=body["params"],
                target=body["target"],
            )
            print("BO has registered: {} points.".format(len(self._bo.space)), end="\n")
        except KeyError:
            pass
        finally:
            suggested_params = self._bo.suggest(self._uf)

        self.write(json.dumps(suggested_params))


def run_optimization_app():
    asyncio.set_event_loop(asyncio.new_event_loop())
    handlers = [
        (r"/bayesian_optimization", BayesianOptimizationHandler),
    ]
    server = tornado.httpserver.HTTPServer(
        tornado.web.Application(handlers)
    )
    server.listen(9009)
    tornado.ioloop.IOLoop.instance().start()


def run_optimizer():
    name = "PID Optimizer"
    # colour = Fore.GREEN

    register_data = {}
    max_target = None
    for _ in range(Number_of_iter):
        status = name + " wants to register: {}.\n".format(register_data)

        resp = requests.post(
            url="http://localhost:9009/bayesian_optimization",
            json=register_data,
        ).json()
        target = black_box_function(**resp)

        register_data = {
            "params": resp,
            "target": target,
        }

        if max_target is None or target > max_target:
            max_target = target

        status += name + " got {} as target.\n".format(target)
        status += name + " will to register next: {}.\n".format(register_data)
        print(status, end="\n")

    global results
    results.append((name, max_target))
    print(name + " is done!", end="\n\n")


if __name__ == "__main__":

    print("welcome to bayesian_optimization on PID Control")

    ser.write(b'1')  # 发送字符 '1'，需要转换为字节类型

    try:
        while True:
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting)
                print("小车匹配成功！")  # 解码为字符串并打印
                answer = data.decode('utf-8')
    except serial.SerialException as e:
        print("串口读取错误:", e)

    ioloop = tornado.ioloop.IOLoop.instance()
    optimizers_config = [
        {"name": "PID Optimizer"},
    ]

    app_thread = threading.Thread(target=run_optimization_app)
    app_thread.daemon = True
    app_thread.start()

    targets = (
        run_optimizer,
    )
    optimizer_threads = []
    for target in targets:
        optimizer_threads.append(threading.Thread(target=target))
        optimizer_threads[-1].daemon = True
        optimizer_threads[-1].start()

    results = []
    for optimizer_thread in optimizer_threads:
        optimizer_thread.join()

    for result in results:
        print(result[0], "found a maximum value of: {}".format(result[1]))

    ioloop.stop()

    ser.close()
