import 'package:polymer/polymer.dart';
import 'dart:html';
import 'dart:async';
import 'dart:convert';

import '../inputzone.dart';

import 'package:paper_elements/paper_dropdown_menu.dart';


@CustomTag('iz-params-base')
class IzParamsBase extends PolymerElement {
  
  @observable FileTask context=null;
  
  ObservableList<Preset> presets = new ObservableList<Preset>();
  int currentPreset =255;
  
  String ouput_ext='';
  
  IzParamsBase.created() : super.created(){
    }
  
  bool _ready=false;
  void ready(){
    if(_ready)
      return;
    _ready=true;
   
  }
  
  void downloadPresets(){
    new Future.delayed(new Duration(milliseconds:300)).then((_){          
    if(context!=null && iz_app.iz_params_content.dataset.containsKey('presets')){
            HttpRequest.getString(iz_app.iz_params_content.dataset['presets']).then((s){
              (JSON.decode(s) as List<Map>).forEach((m)=>presets.add(new Preset.fromMap(m)));
              presets.add(new Preset('Custom settings', ''));
              $['presetmenu'].selected=0;
              onChangePreset();
              });
    }
              });
  }
  
  void onChangePreset(){
    int index = $['presetmenu'].selected;
    if(index==currentPreset)
      return;
    currentPreset=index;
    presetSelected(index, presets[index]);
  }
  
  void presetSelected(int index, Preset preset){
  }
  
  
  void selectCustomPreset(){
    $['presetmenu'].selected = presets.length-1;
  }
  
  String tParam(String base, String otherparams){
    String s = base;
    s= s.replaceAll('%OUTPUT_EXT%', ouput_ext);
    if(s.contains('%PARAMS%'))
      s= s.replaceAll('%PARAMS%', otherparams);
    else
      s+= otherparams;
    return s;
  } 
  
}


class Preset{
  String name;
  String value;
  String output_ext = null;
  Preset.fromMap(Map m){
    name = m['name'];
    value= m['value'];
    if(m.containsKey('output_ext'))
      output_ext=m['output_ext'];
  }
  Preset(this.name,this.value);
}