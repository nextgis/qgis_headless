<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis version="3.44.1-Solothurn" styleCategories="Diagrams">
  <LinearlyInterpolatedDiagramRenderer classificationAttributeExpression="" upperValue="20" upperWidth="10" lowerHeight="0" lowerValue="0" lowerWidth="0" upperHeight="10" diagramType="Histogram" attributeLegend="0">
    <DiagramCategory backgroundColor="#ffffff" penWidth="0" opacity="1" penAlpha="0" stackedDiagramSpacingUnitScale="3x:0,0,0,0,0,0" labelPlacementMethod="XHeight" backgroundAlpha="255" sizeScale="3x:0,0,0,0,0,0" diagramOrientation="Up" sizeType="MM" rotationOffset="270" height="15" direction="0" spacingUnitScale="3x:0,0,0,0,0,0" minScaleDenominator="0" stackedDiagramSpacingUnit="MM" minimumSize="0" barWidth="5" spacing="0" spacingUnit="MM" showAxis="0" scaleBasedVisibility="0" stackedDiagramMode="Horizontal" lineSizeType="MM" enabled="1" maxScaleDenominator="1e+08" penColor="#000000" lineSizeScale="3x:0,0,0,0,0,0" stackedDiagramSpacing="0" width="15" scaleDependency="Area">
      <fontProperties italic="0" description="Ubuntu Sans,11,-1,5,50,0,0,0,0,0" underline="0" strikethrough="0" bold="0" style=""/>
      <attribute color="#ff0000" colorOpacity="1" label="value_1" field="&quot;value_1&quot;"/>
      <attribute color="#00ff00" colorOpacity="1" label="value_2" field="&quot;value_2&quot;"/>
      <axisSymbol>
        <symbol is_animated="0" alpha="1" clip_to_extent="1" force_rhr="0" frame_rate="10" type="line" name="">
          <data_defined_properties>
            <Option type="Map">
              <Option value="" type="QString" name="name"/>
              <Option name="properties"/>
              <Option value="collection" type="QString" name="type"/>
            </Option>
          </data_defined_properties>
          <layer enabled="1" pass="0" id="{e2ab9a9e-1231-4199-8d36-8c3c7756fa9e}" class="SimpleLine" locked="0">
            <Option type="Map">
              <Option value="0" type="QString" name="align_dash_pattern"/>
              <Option value="square" type="QString" name="capstyle"/>
              <Option value="5;2" type="QString" name="customdash"/>
              <Option value="3x:0,0,0,0,0,0" type="QString" name="customdash_map_unit_scale"/>
              <Option value="MM" type="QString" name="customdash_unit"/>
              <Option value="0" type="QString" name="dash_pattern_offset"/>
              <Option value="3x:0,0,0,0,0,0" type="QString" name="dash_pattern_offset_map_unit_scale"/>
              <Option value="MM" type="QString" name="dash_pattern_offset_unit"/>
              <Option value="0" type="QString" name="draw_inside_polygon"/>
              <Option value="bevel" type="QString" name="joinstyle"/>
              <Option value="35,35,35,255,rgb:0.1372549,0.1372549,0.1372549,1" type="QString" name="line_color"/>
              <Option value="solid" type="QString" name="line_style"/>
              <Option value="0.26" type="QString" name="line_width"/>
              <Option value="MM" type="QString" name="line_width_unit"/>
              <Option value="0" type="QString" name="offset"/>
              <Option value="3x:0,0,0,0,0,0" type="QString" name="offset_map_unit_scale"/>
              <Option value="MM" type="QString" name="offset_unit"/>
              <Option value="0" type="QString" name="ring_filter"/>
              <Option value="0" type="QString" name="trim_distance_end"/>
              <Option value="3x:0,0,0,0,0,0" type="QString" name="trim_distance_end_map_unit_scale"/>
              <Option value="MM" type="QString" name="trim_distance_end_unit"/>
              <Option value="0" type="QString" name="trim_distance_start"/>
              <Option value="3x:0,0,0,0,0,0" type="QString" name="trim_distance_start_map_unit_scale"/>
              <Option value="MM" type="QString" name="trim_distance_start_unit"/>
              <Option value="0" type="QString" name="tweak_dash_pattern_on_corners"/>
              <Option value="0" type="QString" name="use_custom_dash"/>
              <Option value="3x:0,0,0,0,0,0" type="QString" name="width_map_unit_scale"/>
            </Option>
            <data_defined_properties>
              <Option type="Map">
                <Option value="" type="QString" name="name"/>
                <Option name="properties"/>
                <Option value="collection" type="QString" name="type"/>
              </Option>
            </data_defined_properties>
          </layer>
        </symbol>
      </axisSymbol>
    </DiagramCategory>
  </LinearlyInterpolatedDiagramRenderer>
  <DiagramLayerSettings priority="5" dist="0" linePlacementFlags="1" placement="0" zIndex="0" showAll="1" obstacle="0">
    <properties>
      <Option type="Map">
        <Option value="" type="QString" name="name"/>
        <Option type="Map" name="properties">
          <Option type="Map" name="show">
            <Option value="true" type="bool" name="active"/>
            <Option value="&quot;value_3&quot; > 10" type="QString" name="expression"/>
            <Option value="3" type="int" name="type"/>
          </Option>
        </Option>
        <Option value="collection" type="QString" name="type"/>
      </Option>
    </properties>
  </DiagramLayerSettings>
  <layerGeometryType>2</layerGeometryType>
</qgis>
