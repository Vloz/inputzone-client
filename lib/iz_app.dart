import 'package:polymer/polymer.dart';
import 'package:paper_elements/paper_tabs.dart';
import 'package:paper_elements/paper_tab.dart';
import 'package:core_elements/core_animated_pages.dart';
import 'package:inputzone/iz_converter.dart';
import 'dart:html';

@CustomTag('iz-app')
class IzApp extends PolymerElement {

  
  
  IzApp.created() : super.created(){  
   generateTabs();
   print(($['pages'] as CoreAnimatedPages).children.first.id.toString());
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
             ($['tabs'] as PaperTabs).selected=($['tabs'] as PaperTabs).children.indexOf(tabConverter);
             ($['pages'] as CoreAnimatedPages).selected = ($['tabs'] as PaperTabs).children.indexOf(tabConverter);
           }
           tabConverter.onClick.listen((_){
             ($['pages'] as CoreAnimatedPages).selected = ($['tabs'] as PaperTabs).children.indexOf(tabConverter);
           });
         });
       }
  }
  
  
}