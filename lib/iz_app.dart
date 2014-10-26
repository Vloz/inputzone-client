import 'package:polymer/polymer.dart';
import 'package:paper_elements/paper_tabs.dart';
import 'package:paper_elements/paper_tab.dart';
import 'package:core_elements/core_animated_pages.dart';
import 'package:inputzone/iz_converter.dart';
import 'dart:html';

import 'inputzone.dart';

@CustomTag('iz-app')
class IzApp extends PolymerElement {
  
  Uri appUrl;
  Map<String,String> options = {};
  RUNTIMETYPE runtime=RUNTIMETYPE.EMSCR;

  
  
  IzApp.created() : super.created(){ 
    
    appUrl = Uri.parse(window.location.href); 
    if(appUrl.hasFragment){
      appUrl.fragment.split(";").forEach((f){
        if(f!=""){
          var opt = f.split("=");
          if(opt.length==2)
            options.addAll({opt[0].toLowerCase():opt[1].toLowerCase()});
          else
            options.addAll({opt[0].toLowerCase():''});
        } 
      });
    }
    
    if(options.containsKey('compatibilitymode')){
       runtime = RUNTIMETYPE.EMSCR;
     }
     else if(window.navigator.mimeTypes.any((mimetype)=> mimetype.type == 'application/x-pnacl')){
       runtime = RUNTIMETYPE.PNACL;
     }   
  
    
  }
  
  void ready(){
    
      generateTabs();
  }
  
  //Create header tabs depending on iz-converter content & select the "active" one
  void generateTabs(){
       if(this.querySelectorAll('iz-converter').length>1){
         this.querySelectorAll('iz-converter').forEach((converter){
           PaperTab tabConverter = new Element.tag('paper-tab');
           tabConverter..attributes['role'] = 'tab'
               ..text = converter.id;

           ($['tabs'] as PaperTabs).children.add(tabConverter);
           if(converter.attributes.keys.contains('active')){
             int index = ($['tabs'] as PaperTabs).children.indexOf(tabConverter);
             ($['tabs'] as PaperTabs).selected=index;
             ($['pages'] as CoreAnimatedPages).selected = index;
           }
           tabConverter.onClick.listen((_){
             int selIndex = ($['tabs'] as PaperTabs).children.indexOf(tabConverter);
             ($['pages'] as CoreAnimatedPages).selected = selIndex;
             ((this.shadowRoot.querySelector('content') as ContentElement).getDistributedNodes()[selIndex] as IzConverter).initConverter();
           });
         });
       }
  }
  
  
}
