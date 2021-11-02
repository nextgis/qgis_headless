<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis readOnly="0" maxScale="0" labelsEnabled="0" version="3.10.4-A Coruña" styleCategories="AllStyleCategories" hasScaleBasedVisibilityFlag="0" simplifyAlgorithm="0" simplifyMaxScale="1" simplifyDrawingHints="1" simplifyLocal="1" minScale="1e+08" simplifyDrawingTol="1">
  <flags>
    <Identifiable>1</Identifiable>
    <Removable>1</Removable>
    <Searchable>1</Searchable>
  </flags>
  <renderer-v2 type="25dRenderer">
    <symbol alpha="1" type="fill" force_rhr="0" name="symbol" clip_to_extent="1">
      <layer locked="1" pass="0" class="SimpleFill" enabled="1">
        <prop k="border_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>
        <prop k="color" v="0,0,255,255"/>
        <prop k="joinstyle" v="bevel"/>
        <prop k="offset" v="0,0"/>
        <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>
        <prop k="offset_unit" v="MM"/>
        <prop k="outline_color" v="35,35,35,255"/>
        <prop k="outline_style" v="solid"/>
        <prop k="outline_width" v="0.26"/>
        <prop k="outline_width_unit" v="MM"/>
        <prop k="style" v="solid"/>
        <effect type="effectStack" enabled="1">
          <effect type="outerGlow">
            <prop k="blend_mode" v="0"/>
            <prop k="blur_level" v="5"/>
            <prop k="blur_unit" v="MM"/>
            <prop k="blur_unit_scale" v="3x:0,0,0,0,0,0"/>
            <prop k="color1" v="0,0,255,255"/>
            <prop k="color2" v="0,255,0,255"/>
            <prop k="color_type" v="0"/>
            <prop k="discrete" v="0"/>
            <prop k="draw_mode" v="2"/>
            <prop k="enabled" v="1"/>
            <prop k="opacity" v="0.5"/>
            <prop k="rampType" v="gradient"/>
            <prop k="single_color" v="255,0,0,255"/>
            <prop k="spread" v="10"/>
            <prop k="spread_unit" v="MapUnit"/>
            <prop k="spread_unit_scale" v="3x:0,0,0,0,0,0"/>
          </effect>
        </effect>
        <data_defined_properties>
          <Option type="Map">
            <Option type="QString" value="" name="name"/>
            <Option name="properties"/>
            <Option type="QString" value="collection" name="type"/>
          </Option>
        </data_defined_properties>
      </layer>
      <layer locked="0" pass="0" class="GeometryGenerator" enabled="1">
        <prop k="SymbolType" v="Fill"/>
        <prop k="geometryModifier" v="order_parts(   extrude(    segments_to_lines( $geometry ),    cos( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height ),    sin( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height )  ),  'distance(  $geometry,  translate(    @map_extent_center,    1000 * @map_extent_width * cos( radians( @qgis_25d_angle + 180 ) ),    1000 * @map_extent_width * sin( radians( @qgis_25d_angle + 180 ) )  ))',  False)"/>
        <data_defined_properties>
          <Option type="Map">
            <Option type="QString" value="" name="name"/>
            <Option name="properties"/>
            <Option type="QString" value="collection" name="type"/>
          </Option>
        </data_defined_properties>
        <symbol alpha="1" type="fill" force_rhr="0" name="@symbol@1" clip_to_extent="1">
          <layer locked="0" pass="0" class="SimpleFill" enabled="1">
            <prop k="border_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>
            <prop k="color" v="0,255,0,255"/>
            <prop k="joinstyle" v="bevel"/>
            <prop k="offset" v="0,0"/>
            <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>
            <prop k="offset_unit" v="MM"/>
            <prop k="outline_color" v="0,255,0,255"/>
            <prop k="outline_style" v="solid"/>
            <prop k="outline_width" v="0.26"/>
            <prop k="outline_width_unit" v="MM"/>
            <prop k="style" v="solid"/>
            <data_defined_properties>
              <Option type="Map">
                <Option type="QString" value="" name="name"/>
                <Option type="Map" name="properties">
                  <Option type="Map" name="fillColor">
                    <Option type="bool" value="true" name="active"/>
                    <Option type="QString" value="set_color_part(   @symbol_color, 'value',  40 + 19 * abs( $pi - azimuth(     point_n( geometry_n($geometry, @geometry_part_num) , 1 ),     point_n( geometry_n($geometry, @geometry_part_num) , 2 )  ) ) )" name="expression"/>
                    <Option type="int" value="3" name="type"/>
                  </Option>
                </Option>
                <Option type="QString" value="collection" name="type"/>
              </Option>
            </data_defined_properties>
          </layer>
        </symbol>
      </layer>
      <layer locked="0" pass="0" class="GeometryGenerator" enabled="1">
        <prop k="SymbolType" v="Fill"/>
        <prop k="geometryModifier" v="translate(  $geometry,  cos( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height ),  sin( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height ))"/>
        <data_defined_properties>
          <Option type="Map">
            <Option type="QString" value="" name="name"/>
            <Option name="properties"/>
            <Option type="QString" value="collection" name="type"/>
          </Option>
        </data_defined_properties>
        <symbol alpha="1" type="fill" force_rhr="0" name="@symbol@2" clip_to_extent="1">
          <layer locked="0" pass="0" class="SimpleFill" enabled="1">
            <prop k="border_width_map_unit_scale" v="3x:0,0,0,0,0,0"/>
            <prop k="color" v="0,0,255,255"/>
            <prop k="joinstyle" v="bevel"/>
            <prop k="offset" v="0,0"/>
            <prop k="offset_map_unit_scale" v="3x:0,0,0,0,0,0"/>
            <prop k="offset_unit" v="MM"/>
            <prop k="outline_color" v="0,0,255,255"/>
            <prop k="outline_style" v="solid"/>
            <prop k="outline_width" v="0.26"/>
            <prop k="outline_width_unit" v="MM"/>
            <prop k="style" v="solid"/>
            <data_defined_properties>
              <Option type="Map">
                <Option type="QString" value="" name="name"/>
                <Option name="properties"/>
                <Option type="QString" value="collection" name="type"/>
              </Option>
            </data_defined_properties>
          </layer>
        </symbol>
      </layer>
    </symbol>
  </renderer-v2>
  <customproperties>
    <property value="0" key="embeddedWidgets/count"/>
    <property key="variableNames">
      <value>qgis_25d_angle</value>
      <value>qgis_25d_height</value>
    </property>
    <property key="variableValues">
      <value>70</value>
      <value>10</value>
    </property>
  </customproperties>
  <blendMode>0</blendMode>
  <featureBlendMode>0</featureBlendMode>
  <layerOpacity>1</layerOpacity>
  <SingleCategoryDiagramRenderer attributeLegend="1" diagramType="Histogram">
    <DiagramCategory rotationOffset="270" penAlpha="255" sizeScale="3x:0,0,0,0,0,0" opacity="1" lineSizeType="MM" width="15" minimumSize="0" penWidth="0" height="15" labelPlacementMethod="XHeight" scaleBasedVisibility="0" scaleDependency="Area" sizeType="MM" backgroundColor="#ffffff" maxScaleDenominator="1e+08" diagramOrientation="Up" lineSizeScale="3x:0,0,0,0,0,0" backgroundAlpha="255" barWidth="5" penColor="#000000" minScaleDenominator="0" enabled="0">
      <fontProperties description="Ubuntu,11,-1,5,50,0,0,0,0,0" style=""/>
    </DiagramCategory>
  </SingleCategoryDiagramRenderer>
  <DiagramLayerSettings placement="1" showAll="1" linePlacementFlags="18" zIndex="0" obstacle="0" priority="0" dist="0">
    <properties>
      <Option type="Map">
        <Option type="QString" value="" name="name"/>
        <Option name="properties"/>
        <Option type="QString" value="collection" name="type"/>
      </Option>
    </properties>
  </DiagramLayerSettings>
  <geometryOptions geometryPrecision="0" removeDuplicateNodes="0">
    <activeChecks/>
    <checkConfiguration type="Map">
      <Option type="Map" name="QgsGeometryGapCheck">
        <Option type="double" value="0" name="allowedGapsBuffer"/>
        <Option type="bool" value="false" name="allowedGapsEnabled"/>
        <Option type="QString" value="" name="allowedGapsLayer"/>
      </Option>
    </checkConfiguration>
  </geometryOptions>
  <fieldConfiguration/>
  <aliases/>
  <excludeAttributesWMS/>
  <excludeAttributesWFS/>
  <defaults/>
  <constraints/>
  <constraintExpressions/>
  <expressionfields/>
  <attributeactions>
    <defaultAction value="{00000000-0000-0000-0000-000000000000}" key="Canvas"/>
  </attributeactions>
  <attributetableconfig sortExpression="" sortOrder="0" actionWidgetStyle="dropDown">
    <columns>
      <column type="actions" width="-1" hidden="1"/>
    </columns>
  </attributetableconfig>
  <conditionalstyles>
    <rowstyles/>
    <fieldstyles/>
  </conditionalstyles>
  <storedexpressions/>
  <editform tolerant="1"></editform>
  <editforminit/>
  <editforminitcodesource>0</editforminitcodesource>
  <editforminitfilepath></editforminitfilepath>
  <editforminitcode><![CDATA[# -*- coding: utf-8 -*-
"""
QGIS forms can have a Python function that is called when the form is
opened.

Use this function to add extra logic to your forms.

Enter the name of the function in the "Python Init function"
field.
An example follows:
"""
from qgis.PyQt.QtWidgets import QWidget

def my_form_open(dialog, layer, feature):
	geom = feature.geometry()
	control = dialog.findChild(QWidget, "MyLineEdit")
]]></editforminitcode>
  <featformsuppress>0</featformsuppress>
  <editorlayout>generatedlayout</editorlayout>
  <editable>
  </editable>
  <labelOnTop>
  </labelOnTop>
  <widgets/>
  <mapTip></mapTip>
  <layerGeometryType>2</layerGeometryType>
</qgis>
