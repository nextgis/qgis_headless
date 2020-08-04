from collections import namedtuple


BandStat = namedtuple("BandStat", ['min', 'max', 'mean'])
ImageStat = namedtuple("ImageStat", ['red', 'green', 'blue', 'alpha'])


def image_stat(image):
    from PIL.ImageStat import Stat  # Optional dependency
    stat = Stat(image)
    return ImageStat(*[
        BandStat(stat.extrema[b][0], stat.extrema[b][1], stat.mean[b])
        for b in range(4)
    ])