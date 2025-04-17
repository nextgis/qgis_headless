<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis version="3.42.1-Münster" styleCategories="Symbology">
  <renderer-v2 forceraster="0" symbollevels="0" referencescale="-1" type="25dRenderer" enableorderby="1">
    <data-defined-properties>
      <Option type="Map">
        <Option type="QString" name="name" value=""/>
        <Option name="properties"/>
        <Option type="QString" name="type" value="collection"/>
      </Option>
    </data-defined-properties>
    <orderby>
      <orderByClause asc="0" nullsFirst="1">distance(  @geometry,  translate(    @map_extent_center,    1000 * @map_extent_width * cos( radians( @qgis_25d_angle + 180 ) ),    1000 * @map_extent_width * sin( radians( @qgis_25d_angle + 180 ) )  ))</orderByClause>
    </orderby>
    <symbol is_animated="0" frame_rate="10" type="fill" clip_to_extent="1" force_rhr="0" name="symbol" alpha="1">
      <data_defined_properties>
        <Option type="Map">
          <Option type="QString" name="name" value=""/>
          <Option name="properties"/>
          <Option type="QString" name="type" value="collection"/>
        </Option>
      </data_defined_properties>
      <layer enabled="1" class="SimpleFill" pass="0" id="{bb479aae-27a6-4698-846d-44151f38ec85}" locked="1">
        <Option type="Map">
          <Option type="QString" name="border_width_map_unit_scale" value="3x:0,0,0,0,0,0"/>
          <Option type="QString" name="color" value="0,0,255,255,rgb:0,0,1,1"/>
          <Option type="QString" name="joinstyle" value="bevel"/>
          <Option type="QString" name="offset" value="0,0"/>
          <Option type="QString" name="offset_map_unit_scale" value="3x:0,0,0,0,0,0"/>
          <Option type="QString" name="offset_unit" value="MM"/>
          <Option type="QString" name="outline_color" value="0,0,0,255,rgb:0,0,0,1"/>
          <Option type="QString" name="outline_style" value="solid"/>
          <Option type="QString" name="outline_width" value="0.26"/>
          <Option type="QString" name="outline_width_unit" value="MM"/>
          <Option type="QString" name="style" value="solid"/>
        </Option>
        <effect enabled="1" type="effectStack">
          <effect type="outerGlow">
            <Option type="Map">
              <Option type="QString" name="blend_mode" value="0"/>
              <Option type="QString" name="blur_level" value="1.3225"/>
              <Option type="QString" name="blur_unit" value="MM"/>
              <Option type="QString" name="blur_unit_scale" value="3x:0,0,0,0,0,0"/>
              <Option type="QString" name="color1" value="69,116,40,255,rgb:0.27058823529411763,0.45490196078431372,0.15686274509803921,1"/>
              <Option type="QString" name="color2" value="188,220,60,255,rgb:0.73725490196078436,0.86274509803921573,0.23529411764705882,1"/>
              <Option type="QString" name="color_type" value="0"/>
              <Option type="QString" name="direction" value="ccw"/>
              <Option type="QString" name="discrete" value="0"/>
              <Option type="QString" name="draw_mode" value="2"/>
              <Option type="QString" name="enabled" value="1"/>
              <Option type="QString" name="opacity" value="0.5"/>
              <Option type="QString" name="rampType" value="gradient"/>
              <Option type="QString" name="single_color" value="17,17,17,255,rgb:0.06666666666666667,0.06666666666666667,0.06666666666666667,1"/>
              <Option type="QString" name="spec" value="rgb"/>
              <Option type="QString" name="spread" value="4"/>
              <Option type="QString" name="spread_unit" value="MapUnit"/>
              <Option type="QString" name="spread_unit_scale" value="3x:0,0,0,0,0,0"/>
            </Option>
          </effect>
        </effect>
        <data_defined_properties>
          <Option type="Map">
            <Option type="QString" name="name" value=""/>
            <Option name="properties"/>
            <Option type="QString" name="type" value="collection"/>
          </Option>
        </data_defined_properties>
      </layer>
      <layer enabled="1" class="GeometryGenerator" pass="0" id="{ac0e6102-5928-4577-b4ec-157eac04a95e}" locked="0">
        <Option type="Map">
          <Option type="QString" name="SymbolType" value="Fill"/>
          <Option type="QString" name="geometryModifier" value="order_parts(   extrude(    segments_to_lines( $geometry ),    cos( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height ),    sin( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height )  ),  'distance(  $geometry,  translate(    @map_extent_center,    1000 * @map_extent_width * cos( radians( @qgis_25d_angle + 180 ) ),    1000 * @map_extent_width * sin( radians( @qgis_25d_angle + 180 ) )  ))',  False)"/>
          <Option type="QString" name="units" value="MapUnit"/>
        </Option>
        <data_defined_properties>
          <Option type="Map">
            <Option type="QString" name="name" value=""/>
            <Option name="properties"/>
            <Option type="QString" name="type" value="collection"/>
          </Option>
        </data_defined_properties>
        <symbol is_animated="0" frame_rate="10" type="fill" clip_to_extent="1" force_rhr="0" name="@symbol@1" alpha="1">
          <data_defined_properties>
            <Option type="Map">
              <Option type="QString" name="name" value=""/>
              <Option name="properties"/>
              <Option type="QString" name="type" value="collection"/>
            </Option>
          </data_defined_properties>
          <layer enabled="1" class="SimpleFill" pass="0" id="{de4683df-ff07-4b73-bcb3-b0cfed89d914}" locked="0">
            <Option type="Map">
              <Option type="QString" name="border_width_map_unit_scale" value="3x:0,0,0,0,0,0"/>
              <Option type="QString" name="color" value="119,119,119,255,rgb:0.46666666666666667,0.46666666666666667,0.46666666666666667,1"/>
              <Option type="QString" name="joinstyle" value="bevel"/>
              <Option type="QString" name="offset" value="0,0"/>
              <Option type="QString" name="offset_map_unit_scale" value="3x:0,0,0,0,0,0"/>
              <Option type="QString" name="offset_unit" value="MM"/>
              <Option type="QString" name="outline_color" value="119,119,119,255,rgb:0.46666666666666667,0.46666666666666667,0.46666666666666667,1"/>
              <Option type="QString" name="outline_style" value="solid"/>
              <Option type="QString" name="outline_width" value="0.26"/>
              <Option type="QString" name="outline_width_unit" value="MM"/>
              <Option type="QString" name="style" value="solid"/>
            </Option>
            <data_defined_properties>
              <Option type="Map">
                <Option type="QString" name="name" value=""/>
                <Option type="Map" name="properties">
                  <Option type="Map" name="fillColor">
                    <Option type="bool" name="active" value="false"/>
                    <Option type="QString" name="expression" value="set_color_part(   @symbol_color, 'value',  40 + 19 * abs( $pi - azimuth(     point_n( geometry_n($geometry, @geometry_part_num) , 1 ),     point_n( geometry_n($geometry, @geometry_part_num) , 2 )  ) ) )"/>
                    <Option type="int" name="type" value="3"/>
                  </Option>
                </Option>
                <Option type="QString" name="type" value="collection"/>
              </Option>
            </data_defined_properties>
          </layer>
        </symbol>
      </layer>
      <layer enabled="1" class="GeometryGenerator" pass="0" id="{2cc53ad4-a294-404d-8a7c-3b0ec42ca0e4}" locked="0">
        <Option type="Map">
          <Option type="QString" name="SymbolType" value="Fill"/>
          <Option type="QString" name="geometryModifier" value="translate(  $geometry,  cos( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height ),  sin( radians( eval( @qgis_25d_angle ) ) ) * eval( @qgis_25d_height ))"/>
          <Option type="QString" name="units" value="MapUnit"/>
        </Option>
        <data_defined_properties>
          <Option type="Map">
            <Option type="QString" name="name" value=""/>
            <Option name="properties"/>
            <Option type="QString" name="type" value="collection"/>
          </Option>
        </data_defined_properties>
        <symbol is_animated="0" frame_rate="10" type="fill" clip_to_extent="1" force_rhr="0" name="@symbol@2" alpha="1">
          <data_defined_properties>
            <Option type="Map">
              <Option type="QString" name="name" value=""/>
              <Option name="properties"/>
              <Option type="QString" name="type" value="collection"/>
            </Option>
          </data_defined_properties>
          <layer enabled="1" class="SimpleFill" pass="0" id="{5bbe4ca9-1c86-446a-bb30-8768839a88b8}" locked="0">
            <Option type="Map">
              <Option type="QString" name="border_width_map_unit_scale" value="3x:0,0,0,0,0,0"/>
              <Option type="QString" name="color" value="177,169,124,255,rgb:0.69411764705882351,0.66274509803921566,0.48627450980392156,1"/>
              <Option type="QString" name="joinstyle" value="bevel"/>
              <Option type="QString" name="offset" value="0,0"/>
              <Option type="QString" name="offset_map_unit_scale" value="3x:0,0,0,0,0,0"/>
              <Option type="QString" name="offset_unit" value="MM"/>
              <Option type="QString" name="outline_color" value="177,169,124,255,rgb:0.69411764705882351,0.66274509803921566,0.48627450980392156,1"/>
              <Option type="QString" name="outline_style" value="solid"/>
              <Option type="QString" name="outline_width" value="0.26"/>
              <Option type="QString" name="outline_width_unit" value="MM"/>
              <Option type="QString" name="style" value="solid"/>
            </Option>
            <data_defined_properties>
              <Option type="Map">
                <Option type="QString" name="name" value=""/>
                <Option name="properties"/>
                <Option type="QString" name="type" value="collection"/>
              </Option>
            </data_defined_properties>
          </layer>
        </symbol>
      </layer>
    </symbol>
  </renderer-v2>
  <selection mode="Default">
    <selectionColor invalid="1"/>
    <selectionSymbol>
      <symbol is_animated="0" frame_rate="10" type="fill" clip_to_extent="1" force_rhr="0" name="" alpha="1">
        <data_defined_properties>
          <Option type="Map">
            <Option type="QString" name="name" value=""/>
            <Option name="properties"/>
            <Option type="QString" name="type" value="collection"/>
          </Option>
        </data_defined_properties>
        <layer enabled="1" class="SimpleFill" pass="0" id="{ccfdf650-f10f-4ce1-8ba9-1963ba45ea12}" locked="0">
          <Option type="Map">
            <Option type="QString" name="border_width_map_unit_scale" value="3x:0,0,0,0,0,0"/>
            <Option type="QString" name="color" value="0,0,255,255,rgb:0,0,1,1"/>
            <Option type="QString" name="joinstyle" value="bevel"/>
            <Option type="QString" name="offset" value="0,0"/>
            <Option type="QString" name="offset_map_unit_scale" value="3x:0,0,0,0,0,0"/>
            <Option type="QString" name="offset_unit" value="MM"/>
            <Option type="QString" name="outline_color" value="35,35,35,255,rgb:0.13725490196078433,0.13725490196078433,0.13725490196078433,1"/>
            <Option type="QString" name="outline_style" value="solid"/>
            <Option type="QString" name="outline_width" value="0.26"/>
            <Option type="QString" name="outline_width_unit" value="MM"/>
            <Option type="QString" name="style" value="solid"/>
          </Option>
          <data_defined_properties>
            <Option type="Map">
              <Option type="QString" name="name" value=""/>
              <Option name="properties"/>
              <Option type="QString" name="type" value="collection"/>
            </Option>
          </data_defined_properties>
        </layer>
      </symbol>
    </selectionSymbol>
  </selection>
  <blendMode>0</blendMode>
  <featureBlendMode>0</featureBlendMode>
  <layerGeometryType>2</layerGeometryType>
</qgis>
