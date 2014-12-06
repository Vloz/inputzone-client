import 'package:polymer/polymer.dart';
import 'dart:html';
import 'dart:async';

import '../inputzone.dart';
import 'iz_params_base.dart';

import 'package:paper_elements/paper_input.dart';
import 'package:paper_elements/paper_slider.dart';
import 'package:paper_elements/paper_radio_button.dart';
import 'package:paper_elements/paper_radio_group.dart';
import 'package:paper_elements/paper_toggle_button.dart';

@CustomTag('iz-params')
class IzParams extends IzParamsBase with Polymer, Observable {
  
  List<Option> _options = new List<Option>();

  
  IzParams.created() : super.created(){
  }
  
  void ready(){
    super.ready();
    downloadPresets();
    new Future.delayed(new Duration(milliseconds:600)).then((_){
           setupOptions();
          });
  }
  
  
  void presetSelected(int index, Preset preset){
    if(index != presets.length-1)
    {
      _options.clear();
      if(preset.output_ext!=null)
        ouput_ext=preset.output_ext;
      context.params= "./"+iz_app.name+" "+tParam(context.baseParams,preset.value);
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
            selectCustomPreset();
            updateParams();
        });
          break;
        case 'paper-input':
          (node as PaperInput).onInput.listen((o){
          updateElementOption(o.target,(o.target as PaperInput).inputValue.trim());
          selectCustomPreset();
          updateParams();
        });
          break;
        case 'paper-radio-button':
          (node as PaperRadioButton).on['core-change'].listen((o){
          updateElementOption(o.target,((o as CustomEvent).target as PaperRadioButton).checked?" ":"");
          selectCustomPreset();
          updateParams();
        });
          break;
        case 'paper-toggle-button':
          (node as PaperToggleButton).on['core-change'].listen((o){
            updateElementOption(o.target,((o as CustomEvent).target as PaperToggleButton).checked?" ":"");
            selectCustomPreset();
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
  
  
  void updateParams(){
    String s='';
    _options.sort((o1,o2)=>o1.position.compareTo(o2.position));
    _options.forEach((option)=>s+= option.toString());
    context.params=tParam("./"+iz_app.name+" "+context.baseParams+" ", s);
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
