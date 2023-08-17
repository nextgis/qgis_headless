import io
from argparse import ArgumentParser

from qgis_headless import CRS, Layer, MapRequest, Style, deinit, init


def main():
    parser = ArgumentParser()
    parser.add_argument('-e', '--extent', type=float, nargs=4, required=True, metavar=('MIN_X', 'MIN_Y', 'MAX_X', 'MAX_Y'))
    parser.add_argument('-s', '--size', type=int, nargs=2, required=True, metavar=('WIDTH', 'HEIGHT'))
    parser.add_argument('-c', '--crs', type=int, default=3857, metavar='EPSG_CODE')
    parser.add_argument('-r', '--repeat', type=int, default=1)
    parser.add_argument('-o', '--output', type=str, default=None)
    parser.add_argument('data')
    parser.add_argument('qml')

    args = parser.parse_args()

    init([])

    for i in range(args.repeat):
        req = MapRequest()
        req.set_dpi(96)
        req.set_crs(CRS.from_epsg(args.crs))

        req.add_layer(
            Layer.from_ogr(args.data),
            Style.from_file(args.qml))

        image = req.render_image(tuple(args.extent), tuple(args.size))

    if args.output is not None:
        with io.open(args.output, 'wb') as fd:
            fd.write(image.to_bytes())

    deinit()