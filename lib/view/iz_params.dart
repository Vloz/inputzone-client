import 'package:polymer/polymer.dart';
import 'dart:html';
import 'dart:async';
import 'dart:convert';

import '../inputzone.dart';

import 'package:paper_elements/paper_input.dart';
import 'package:paper_elements/paper_slider.dart';
import 'package:paper_elements/paper_radio_button.dart';
import 'package:paper_elements/paper_radio_group.dart';
import 'package:paper_elements/paper_dropdown_menu.dart';
import 'package:paper_elements/paper_toggle_button.dart';

@CustomTag('iz-params')
class IzParams extends PolymerElement {
  
  @observable FileTask context=null;
  
  List<Option> _options = new List<Option>();
  ObservableList<Preset> presets = new ObservableList<Preset>();
  
  PaperDropdownMenu presetMenu;
  String ouput_ext='';

  
  IzParams.created() : super.created(){
  }
  
  bool _ready=false;
  void ready(){
    if(_ready)
      return;
    _ready=true;
    presetMenu=$['presetmenu'];
    
    if(iz_app.iz_params_content.dataset.containsKey('presets')){
      HttpRequest.getString(iz_app.iz_params_content.dataset['presets']).then((s){
        (JSON.decode(s) as List<Map>).forEach((m)=>presets.add(new Preset.fromMap(m)));
        presets.add(new Preset('Custom settings', ''));
        presetMenu.selected=0;
        setupOptions();
        onChangePreset();
        });
    }
    else
      new Future.delayed(new Duration(milliseconds:300)).then((_){
       setupOptions();
      });
    
   
  }
  int currentPreset =255;
  void onChangePreset(){
    int index = presetMenu.selected;
    if(index==currentPreset)
      return;
    currentPreset=index;
    if(index != presets.length-1)
    {
      _options.clear();
      if(presets[index].output_ext!=null)
        ouput_ext=presets[index].output_ext;
      context.params= "./"+iz_app.name+" "+tParam(context.baseParams,presets[index].value);
      updateFromText();
    }
  }
  
  void updateFromText(){
    //Reint boolean controllers to deal with flag now missing
    (shadowRoot.querySelector('content') as ContentElement).getDistributedNodes().first.parent.querySelectorAll("paper-toggle-button,paper-radio-button").forEach((e){
      switch(e.tagName.toLowerCase()){
        case "paper-toggle-button":
          (e as PaperToggleButton).checked = false;
          break;
        case "paper-radio-button":
          if(e.parent is! PaperRadioButton)
            (e as PaperRadioButton).checked = false;
          break;
      }
    });
    List<String> parts = new List<String>();
    RegExp exp = new RegExp("[^\\s\"']+|\"([^\"]*)\"");
    String p ="";
    if(context.baseParams.contains('%PARAMS%')){
      int lastPartlength = context.baseParams.length - (tParam(context.baseParams,'%PARAMS%').indexOf("%PARAMS%")+8);
      p= context.params.substring(tParam(context.baseParams,'%PARAMS%').indexOf("%PARAMS%"), context.params.length - lastPartlength);
    } else
      p = context.params.substring(tParam(context.baseParams,'').length);
      
    exp.allMatches(p).forEach((m){
      if (m.group(1) != null)
        parts.add(m.group(1).trim());// Add double-quoted string without the quotes
      else
        parts.add(m.group(0).trim());// Add unquoted word          
    });
    
    for(int i=0;i<parts.length;i++){
      Element element = null;
      String flag='';
      String newValue = '';
      if(parts[i].startsWith('-')){
        flag = parts[i];;
        bool isBoolean = false;
        if(i+1>=parts.length ||parts[i+1].startsWith('-')){
          newValue=' ';
          isBoolean = true;
        }
        else
          newValue = parts[++i];
        if(_options.any((o)=>o.flag==flag))
          _options.where((o)=>o.flag==flag).first.value=newValue;
        else
          _options.add(new Option(newValue,flag:flag)..isBoolean=isBoolean);
        element = (shadowRoot.querySelector('content') as ContentElement).getDistributedNodes().first.parent.querySelector("[data-option-flag='"+flag+"']");        
      }
      else{
        if(_options.any((o)=>o.position==i))
          _options.where((o)=>o.position==i).first.value=newValue;
        else
          _options.add(new Option(newValue,position:i));
        element = (shadowRoot.querySelector('content') as ContentElement).getDistributedNodes().first.parent.querySelector("[data-option-pos='"+i.toString()+"']");
      }    
      
      if(element!=null){
        switch(element.tagName.toLowerCase()){
          case 'paper-slider':
            (element as PaperSlider).value = int.parse(newValue);
            break;
          case 'paper-input':
            (element as PaperInput).inputValue = newValue;
            break;
          case 'paper-radio-button':
            if(element.parent is PaperRadioGroup)
              (element.parent as PaperRadioGroup).selected = flag;
            else
              (element as PaperRadioButton).checked = true;
          
            break;
          case 'paper-toggle-button':
            (element as PaperToggleButton).checked = true;
            break;
          default:
            print('Unknown tag name '+element.tagName+' in iz-params updateFromText!');
          
        }
        
      }
    }
  }
  
  
  void setupOptions(){
    
    (shadowRoot.querySelector('content') as ContentElement).getDistributedNodes().first.parent.querySelectorAll('paper-slider,paper-input,paper-radio-button,paper-toggle-button').forEach((node){
      switch((node as Element).tagName.toLowerCase()){
        case 'paper-slider':
          (node as PaperSlider).onChange.listen((o){
            updateElementOption(o.target,(o.target as PaperSlider).value.toString());
            setOnCustomPreset();
            updateParams();
        });
          break;
        case 'paper-input':
          (node as PaperInput).onInput.listen((o){
          updateElementOption(o.target,(o.target as PaperInput).inputValue.trim());
          setOnCustomPreset();
          updateParams();
        });
          break;
        case 'paper-radio-button':
          (node as PaperRadioButton).on['core-change'].listen((o){
          updateElementOption(o.target,((o as CustomEvent).target as PaperRadioButton).checked?" ":"");
          setOnCustomPreset();
          updateParams();
        });
          break;
        case 'paper-toggle-button':
          (node as PaperToggleButton).on['core-change'].listen((o){
            updateElementOption(o.target,((o as CustomEvent).target as PaperToggleButton).checked?" ":"");
            setOnCustomPreset();
            updateParams();
          });
          break;
        
      }
    });
  }
  
  void updateElementOption(Element o,var value){
    if(o.dataset.containsKey('optionFlag')){
      if(_options.any((opt)=>opt.flag==o.dataset['optionFlag']))
        _options.firstWhere((opt)=>opt.flag==o.dataset['optionFlag']).value = value;
      else
        _options.add(new Option(value,flag: o.dataset['optionFlag'],element:o));
    }else if(o.dataset.containsKey('optionPos')){
      int pos = int.parse(o.dataset['optionPos']);
      if(_options.any((opt)=>opt.position==pos))
        _options.firstWhere((opt)=>opt.flag==pos).value = value;
      else
        _options.add(new Option(value,position: pos));
    }
  }
//            
//            (shadowRoot.querySelector('content') as ContentElement).getDistributedNodes().first.parent.querySelectorAll('paper-radio-button').forEach((node){
//                      var newOption = createOption(node);
//                      (node as PaperRadioButton).on['core-change'].listen((o){
//                        newOption.value = ((o as CustomEvent).target as PaperRadioButton).checked?" ":""; 
//                        updateParams();
//                      });
//                                  
//                });
  
  
  void setOnCustomPreset(){
    presetMenu.selected = presets.length-1;
  }
  
//  Option createOption(HtmlElement node){
//    int pos = 256;
//    String flag ='';
//    if(node.dataset.containsKey('optionFlag'))
//      flag=node.dataset['optionFlag'];
//    if(node.dataset.containsKey('optionPos'))
//      pos=int.parse(node.dataset['optionPos']);
//    Option newOption = new Option(flag: flag,position: pos); 
//    if(node.dataset.containsKey('isboolean'))
//      newOption.isBoolean =true; 
//    _options.add(newOption);
//    return newOption;
//  }
  
  void updateParams(){
    String s='';
    _options.sort((o1,o2)=>o1.position.compareTo(o2.position));
    _options.forEach((option)=>s+= option.toString());
    context.params=tParam("./"+iz_app.name+" "+context.baseParams+" ", s);
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

class Option{
  String flag='';
  String value='';
  bool isBoolean=false;
  int position;
  Option(this.value,{Element element:null,this.flag:'',this.position:256}){
    if(element!=null)
      isBoolean = element is PaperRadioButton;
  }
  String toString(){
    if(value=='')
      return'';
    return (flag+' '+value).trim()+' ';
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