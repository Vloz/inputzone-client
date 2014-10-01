import 'package:polymer/polymer.dart';
import 'dart:html';
import 'dart:js';
import 'inputzone.dart';
import 'iz_output_list.dart';

@CustomTag('iz-converter')
class IzConverter extends PolymerElement {

  @published String title = "Title";
  @published String subtitle = "A short Subtitle"; 
  @published String tabname = "tabname";
  
  EmbedElement embed;
  JsObject naclproxy;
  IzOutputList outputlist;

  
  IzConverter.created() : super.created(){  
    embed= this.querySelector("embed");
    embed.onLoad.listen((_){
          embed.on['message'].listen((s)=>print((s as MessageEvent).data.toString()));
          naclproxy = new JsObject.fromBrowserObject(embed); 
        });

    
  }
  
  ready(){
    outputlist = $['outputlist'];
  }
  
  
  void clickZone(){
    var task = new FileTask("test");
    outputlist.addTask(task);
//    InputElement input = new InputElement(type:'file');
//    input.onChange.first.then((evt)=>handleFile(evt));
//    input.click();
  }
  
}