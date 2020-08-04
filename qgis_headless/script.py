import sys
import io
from argparse import ArgumentParser

import qgis_headless


def main():
    parser = ArgumentParser()
    parser.add_argument('-e', '--extent', type=float, nargs=4, required=True, metavar=('MIN_X', 'MIN_Y', 'MAX_X', 'MAX_Y'))
    parser.add_argument('-s', '--size', type=int, nargs=2, required=True, metavar=('WIDTH', 'HEIGHT'))
    parser.add_argument('-c', '--crs', type=int, default=3187, metavar='EPSG_CODE')
    parser.add_argument('-r', '--repeat', type=int, default=1)
    parser.add_argument('-o', '--output', type=str, default=None)
    parser.add_argument('data')
    parser.add_argument('qml')

    args = parser.parse_args()

    qgis_headless.init([])

    with io.open(args.qml, 'rb') as fd:
        style = fd.read()

    fargs = (args.data, style, ) + tuple(args.extent) + tuple(args.size) + (args.crs, -1)

    for i in range(args.repeat):
        data = qgis_headless.renderVector(*fargs)

    if args.output is not None:
        with io.open(args.output, 'wb') as fd:
            fd.write(data.to_string())

    qgis_headless.deinit()