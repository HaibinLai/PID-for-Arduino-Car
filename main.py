import asyncio
import math
import os
# import opentuner
import re
import threading
import time

from bayes_opt import BayesianOptimization
from bayes_opt.util import UtilityFunction



# from colorama import Fore

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
file_path = '/work/ssc-laihb/haibin/hpl-2.3/testing'
WAITING_TIME = 320
COMPILE_TIME = 60
LSF_TIME = 20

# for bayesian
Kappa = 3
Xi = 1


def black_box_function(N_rate, NBs_rate, NBMIN, BCAST):
    """Function with unknown internals we wish to maximize.

    This is just serving as an example, however, for all intents and
    purposes think of the internals of this function, i.e.: the process
    which generates its outputs values, as unknown.
    """


    return N_rate+NBs_rate-NBMIN-BCAST


class BayesianOptimizationHandler(RequestHandler):
    """Basic functionality for NLP handlers."""
    HPL_para = {"N_rate": (80, 100), "NBs_rate": (0, 100), "NBMIN": (2, 15), "BCAST": (0, 5)}
    _bo = BayesianOptimization(
        f=black_box_function,
        pbounds=HPL_para
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
            print("BO has registered: {} points.".format(len(self._bo.space)), end="\n\n")
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

    print("welcome to bayesian_optimization on HPL")


    time.sleep(COMPILE_TIME)

    ioloop = tornado.ioloop.IOLoop.instance()
    optimizers_config = [
        {"name": "HPL Optimizer"},
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
