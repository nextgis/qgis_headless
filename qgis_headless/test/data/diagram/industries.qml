<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis maxScale="0" simplifyMaxScale="1" simplifyAlgorithm="0" hasScaleBasedVisibilityFlag="0" simplifyLocal="1" simplifyDrawingHints="0" simplifyDrawingTol="1" styleCategories="AllStyleCategories" minScale="1e+08" readOnly="0" version="3.10.4-A Coruña" labelsEnabled="0">
  <flags>
    <Identifiable>1</Identifiable>
    <Removable>1</Removable>
    <Searchable>1</Searchable>
  </flags>
  <renderer-v2 type="nullSymbol"/>
  <customproperties>
    <property key="dualview/previewExpressions">
      <value>"NAME"</value>
    </property>
    <property key="embeddedWidgets/count" value="0"/>
    <property key="variableNames"/>
    <property key="variableValues"/>
  </customproperties>
  <blendMode>0</blendMode>
  <featureBlendMode>0</featureBlendMode>
  <layerOpacity>1</layerOpacity>
  <LinearlyInterpolatedDiagramRenderer upperHeight="50" attributeLegend="1" diagramType="Pie" upperValue="15" lowerHeight="0" classificationField="total" lowerWidth="0" upperWidth="50" lowerValue="0">
    <DiagramCategory minimumSize="0" rotationOffset="270" penWidth="0.2" minScaleDenominator="0" penColor="#004de6" lineSizeScale="3x:0,0,0,0,0,0" barWidth="5" width="15" opacity="1" scaleDependency="Area" backgroundAlpha="255" labelPlacementMethod="XHeight" enabled="1" maxScaleDenominator="1e+08" penAlpha="0" lineSizeType="MM" sizeScale="3x:0,0,0,0,0,0" sizeType="MM" height="15" diagramOrientation="Up" scaleBasedVisibility="0" backgroundColor="#ffffff">
      <fontProperties description="MS Shell Dlg 2,7.8,-1,5,50,0,0,0,0,0" style=""/>
      <attribute field="zern" color="#fedd4a" label="Зерновые и зернобобовые культуры"/>
      <attribute field="ovosch" color="#8be781" label="Овощеводства"/>
      <attribute field="sad" color="#ff85cc" label="Садоводство"/>
      <attribute field="vinograd" color="#b16dff" label="Виноградарство"/>
      <attribute field="efir" color="#67faff" label="Эфиромасличные и лекарственные культуры"/>
      <attribute field="skotovod" color="#cc6114" label="Скотоводство"/>
      <attribute field="svinovod" color="#aab5b9" label="Свиноводство"/>
      <attribute field="ptitcevod" color="#29bbff" label="Птицеводство"/>
    </DiagramCategory>
    <data-defined-size-legend title="" type="separated" valign="center">
      <symbol name="source" type="marker" clip_to_extent="1" alpha="1" force_rhr="0">
        <layer locked="0" pass="0" class="SimpleMarker" enabled="1">
          <prop v="0" k="angle"/>
          <prop v="255,255,255,255" k="color"/>
          <prop v="1" k="horizontal_anchor_point"/>
          <prop v="bevel" k="joinstyle"/>
          <prop v="circle" k="name"/>
          <prop v="0,0" k="offset"/>
          <prop v="3x:0,0,0,0,0,0" k="offset_map_unit_scale"/>
          <prop v="MM" k="offset_unit"/>
          <prop v="35,35,35,255" k="outline_color"/>
          <prop v="solid" k="outline_style"/>
          <prop v="0.2" k="outline_width"/>
          <prop v="3x:0,0,0,0,0,0" k="outline_width_map_unit_scale"/>
          <prop v="MM" k="outline_width_unit"/>
          <prop v="diameter" k="scale_method"/>
          <prop v="2" k="size"/>
          <prop v="3x:0,0,0,0,0,0" k="size_map_unit_scale"/>
          <prop v="MM" k="size_unit"/>
          <prop v="1" k="vertical_anchor_point"/>
          <data_defined_properties>
            <Option type="Map">
              <Option name="name" value="" type="QString"/>
              <Option name="properties"/>
              <Option name="type" value="collection" type="QString"/>
            </Option>
          </data_defined_properties>
        </layer>
      </symbol>
      <text-style color="0,0,0,255" align="1">
        <font italic="0" family="MS Shell Dlg 2" size="8" weight="50"/>
      </text-style>
      <classes>
        <class label="" size="1"/>
        <class label="малые" size="2"/>
        <class label="" size="4"/>
        <class label="средние" size="6"/>
        <class label="крупные" size="15"/>
      </classes>
    </data-defined-size-legend>
  </LinearlyInterpolatedDiagramRenderer>
  <DiagramLayerSettings placement="1" linePlacementFlags="18" obstacle="0" dist="0" zIndex="0" priority="0" showAll="1">
    <properties>
      <Option type="Map">
        <Option name="name" value="" type="QString"/>
        <Option name="properties"/>
        <Option name="type" value="collection" type="QString"/>
      </Option>
    </properties>
  </DiagramLayerSettings>
  <geometryOptions geometryPrecision="0" removeDuplicateNodes="0">
    <activeChecks/>
    <checkConfiguration/>
  </geometryOptions>
  <fieldConfiguration>
    <field name="zern">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="ovosch">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="sad">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="vinograd">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="efir">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="skotovod">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="svinovod">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="ptitcevod">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
    <field name="total">
      <editWidget type="Range">
        <config>
          <Option/>
        </config>
      </editWidget>
    </field>
  </fieldConfiguration>
  <aliases>
    <alias field="zern" name="" index="0"/>
    <alias field="ovosch" name="" index="1"/>
    <alias field="sad" name="" index="2"/>
    <alias field="vinograd" name="" index="3"/>
    <alias field="efir" name="" index="4"/>
    <alias field="skotovod" name="" index="5"/>
    <alias field="svinovod" name="" index="6"/>
    <alias field="ptitcevod" name="" index="7"/>
    <alias field="total" name="" index="8"/>
  </aliases>
  <excludeAttributesWMS/>
  <excludeAttributesWFS/>
  <defaults>
    <default field="zern" applyOnUpdate="0" expression=""/>
    <default field="ovosch" applyOnUpdate="0" expression=""/>
    <default field="sad" applyOnUpdate="0" expression=""/>
    <default field="vinograd" applyOnUpdate="0" expression=""/>
    <default field="efir" applyOnUpdate="0" expression=""/>
    <default field="skotovod" applyOnUpdate="0" expression=""/>
    <default field="svinovod" applyOnUpdate="0" expression=""/>
    <default field="ptitcevod" applyOnUpdate="0" expression=""/>
    <default field="total" applyOnUpdate="0" expression=""/>
  </defaults>
  <constraints>
    <constraint field="zern" exp_strength="0" unique_strength="0" constraints="0" notnull_strength="0"/>
    <constraint field="ovosch" exp_strength="0" unique_strength="0" constraints="0" notnull_strength="0"/>
    <constraint field="sad" exp_strength="0" unique_strength="0" constraints="0" notnull_strength="0"/>
    <constraint field="vinograd" exp_strength="0" unique_strength="0" constraints="0" notnull_strength="0"/>
    <constraint field="efir" exp_strength="0" unique_strength="0" constraints="0" notnull_strength="0"/>
    <constraint field="skotovod" exp_strength="0" unique_strength="0" constraints="0" notnull_strength="0"/>
    <constraint field="svinovod" exp_strength="0" unique_strength="0" constraints="0" notnull_strength="0"/>
    <constraint field="ptitcevod" exp_strength="0" unique_strength="0" constraints="0" notnull_strength="0"/>
    <constraint field="total" exp_strength="0" unique_strength="0" constraints="0" notnull_strength="0"/>
  </constraints>
  <constraintExpressions>
    <constraint field="zern" exp="" desc=""/>
    <constraint field="ovosch" exp="" desc=""/>
    <constraint field="sad" exp="" desc=""/>
    <constraint field="vinograd" exp="" desc=""/>
    <constraint field="efir" exp="" desc=""/>
    <constraint field="skotovod" exp="" desc=""/>
    <constraint field="svinovod" exp="" desc=""/>
    <constraint field="ptitcevod" exp="" desc=""/>
    <constraint field="total" exp="" desc=""/>
  </constraintExpressions>
  <expressionfields/>
  <attributeactions>
    <defaultAction key="Canvas" value="{00000000-0000-0000-0000-000000000000}"/>
  </attributeactions>
  <attributetableconfig actionWidgetStyle="dropDown" sortOrder="1" sortExpression="&quot;Total&quot;">
    <columns>
      <column width="-1" type="actions" hidden="1"/>
      <column name="zern" width="-1" type="field" hidden="0"/>
      <column name="ovosch" width="-1" type="field" hidden="0"/>
      <column name="sad" width="-1" type="field" hidden="0"/>
      <column name="vinograd" width="-1" type="field" hidden="0"/>
      <column name="efir" width="-1" type="field" hidden="0"/>
      <column name="skotovod" width="-1" type="field" hidden="0"/>
      <column name="svinovod" width="-1" type="field" hidden="0"/>
      <column name="ptitcevod" width="-1" type="field" hidden="0"/>
      <column name="total" width="-1" type="field" hidden="0"/>
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
В формах QGIS могут быть использованы функции Python, вызываемые при открытии формы.
Используйте эту возможность, чтобы добавить дополнительную логику к формам.

Введите имя функции в поле "функция инициализации Python".
Пример:
"""
from qgis.PyQt.QtWidgets import QWidget

def my_form_open(dialog, layer, feature):
	geom = feature.geometry()
	control = dialog.findChild(QWidget, "MyLineEdit")
]]></editforminitcode>
  <featformsuppress>0</featformsuppress>
  <editorlayout>generatedlayout</editorlayout>
  <editable>
    <field name="Efir" editable="1"/>
    <field name="NAME" editable="1"/>
    <field name="OFFICIAL_S" editable="1"/>
    <field name="OSM_ID" editable="1"/>
    <field name="Ovosch" editable="1"/>
    <field name="Pop1989" editable="1"/>
    <field name="Pop2001" editable="1"/>
    <field name="Pop2014" editable="1"/>
    <field name="Ptitcevod" editable="1"/>
    <field name="Sad" editable="1"/>
    <field name="Skotovod" editable="1"/>
    <field name="Status_" editable="1"/>
    <field name="Svinovod" editable="1"/>
    <field name="Total" editable="1"/>
    <field name="Vinograd" editable="1"/>
    <field name="Zern" editable="1"/>
    <field name="adm_region" editable="1"/>
    <field name="efir" editable="1"/>
    <field name="ovosch" editable="1"/>
    <field name="ptitcevod" editable="1"/>
    <field name="sad" editable="1"/>
    <field name="skotovod" editable="1"/>
    <field name="svinovod" editable="1"/>
    <field name="total" editable="1"/>
    <field name="vinograd" editable="1"/>
    <field name="zern" editable="1"/>
  </editable>
  <labelOnTop>
    <field name="Efir" labelOnTop="0"/>
    <field name="NAME" labelOnTop="0"/>
    <field name="OFFICIAL_S" labelOnTop="0"/>
    <field name="OSM_ID" labelOnTop="0"/>
    <field name="Ovosch" labelOnTop="0"/>
    <field name="Pop1989" labelOnTop="0"/>
    <field name="Pop2001" labelOnTop="0"/>
    <field name="Pop2014" labelOnTop="0"/>
    <field name="Ptitcevod" labelOnTop="0"/>
    <field name="Sad" labelOnTop="0"/>
    <field name="Skotovod" labelOnTop="0"/>
    <field name="Status_" labelOnTop="0"/>
    <field name="Svinovod" labelOnTop="0"/>
    <field name="Total" labelOnTop="0"/>
    <field name="Vinograd" labelOnTop="0"/>
    <field name="Zern" labelOnTop="0"/>
    <field name="adm_region" labelOnTop="0"/>
    <field name="efir" labelOnTop="0"/>
    <field name="ovosch" labelOnTop="0"/>
    <field name="ptitcevod" labelOnTop="0"/>
    <field name="sad" labelOnTop="0"/>
    <field name="skotovod" labelOnTop="0"/>
    <field name="svinovod" labelOnTop="0"/>
    <field name="total" labelOnTop="0"/>
    <field name="vinograd" labelOnTop="0"/>
    <field name="zern" labelOnTop="0"/>
  </labelOnTop>
  <widgets/>
  <previewExpression>"NAME"</previewExpression>
  <mapTip></mapTip>
  <layerGeometryType>0</layerGeometryType>
</qgis>
