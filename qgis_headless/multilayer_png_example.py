import pathlib
import io,os
#from qgis_headless import MapRequest, CRS, Layer, Style, set_svg_paths, get_qgis_version, init
import qgis_headless 

from qgis_headless.util import  render_vector, to_pil


'''
Sample of qgis_headless render multi-layer image.

docker run -it -v c:\trolleway\1126_qgisrender\repo\nextgis_extracts_picture\test\:/code  -v c:\trolleway\1126_qgisrender\zips\manual\:/data  qgis_headless:focal python3 /code/multilayer_png_example.py

'''
'''
Хотел сделать расчёт охвата по слою, но в контейнере не хватает зависимостей, отложим
'''
#For layer extent
#from osgeo import ogr
#from osgeo import osr
#from pyproj import Proj, transform


def get_bbox(filepath):
    
    ds = gdal.OpenEx(filepath,gdal.OF_READONLY)
    assert ds is not None
    layer = ds.GetLayer()
    assert layer is not None
    extent = layer.GetExtent()
    
    # Create a Polygon from the extent tuple
    ring = ogr.Geometry(ogr.wkbLinearRing)
    ring.AddPoint(extent[0],extent[2])
    ring.AddPoint(extent[1], extent[2])
    ring.AddPoint(extent[1], extent[3])
    ring.AddPoint(extent[0], extent[3])
    ring.AddPoint(extent[0],extent[2])
    poly = ogr.Geometry(ogr.wkbPolygon)
    poly.AddGeometry(ring)

    extent = poly.Buffer(0.7).GetEnvelope()
    
    lx = extent[0]
    ly = extent[2]
    ux = extent[1]
    uy = extent[3]
   
    bbox_string = '{lx},{ly},{ux},{uy}'.format(lx=lx,ly=ly,ux=ux,uy=uy)
    bbox = (lx,ly,rx,ry)
    return bbox  

def reproject_4326_3857(x,y):
 
    inProj = Proj('epsg:4326')
    outProj = Proj('epsg:3857')
   
    xr,yr = transform(inProj,outProj,x,y)
    
    return xr,yr
def bbox4326_3857(bbox):
    lx,ly = reproject_4326_3857(bbox[0],bbox[2])
    ux,uy = reproject_4326_3857(bbox[1],bbox[3])
    bbox_reprojected = (lx,ly,rx,ry)
    return bbox_reprojected
        

qgis_headless.init([])
req = qgis_headless.MapRequest()
req.set_crs(qgis_headless.CRS.from_epsg(3857))

layer_path = os.path.join('/data/extract','data', 'settlement-point.shp')
style_path = pathlib.Path(os.path.join('/data/qmls/','settlement-point.qml'))
assert os.path.isfile(layer_path)
assert os.path.isfile(style_path)
req.add_layer(qgis_headless.Layer.from_ogr(layer_path), qgis_headless.Style.from_string(style_path.read_text()))

layer_path = os.path.join('/data/extract','data', 'highway-line.shp')
style_path = pathlib.Path(os.path.join('/data/qmls/','highway-line.qml'))
assert os.path.isfile(layer_path)
assert os.path.isfile(style_path)
req.add_layer(qgis_headless.Layer.from_ogr(layer_path), qgis_headless.Style.from_string(style_path.read_text()))

extent = (3095455.0, 8256741.0, 3636672.0, 8540270.0)
#extent = bbox4326_3857(get_bbox(layer_path))


image = req.render_image(extent, (800,800))
qgis_headless.util.to_pil(image).save("/code/i.png")
