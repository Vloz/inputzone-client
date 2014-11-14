import 'package:polymer/polymer.dart';
import 'dart:html';
import 'dart:convert';
import 'inputzone.dart';

@CustomTag('iz-register')
class IzRegister extends PolymerElement {
  @published bool minify = false;
  
  List<RegisterEntry> allEntries = new List<RegisterEntry>();
  @observable ObservableList<RegisterEntry> entries = new ObservableList<RegisterEntry>();
  
  IzRegister.created() : super.created(){
  }
  void ready(){
    HttpRequest.getString("/register.json").then((s){
      JSON.decode(s).forEach((e)=>allEntries.add(new RegisterEntry.fromJSON(e)));
      filter();
    });
  }
  
  void filter([String fragment='']){
    entries.clear();
    if(fragment=='')
      entries.addAll(allEntries);
    else{
      String f = fragment.toLowerCase();
      entries.addAll(
          allEntries.where((e)=>e.cname.toLowerCase().contains(f) 
          || e.intag.any((tag)=>tag.toLowerCase().contains(f)) 
          || e.outag.any((tag)=>tag.toLowerCase().contains(f)))
          );
    }
  }
  
  void onEntryClick(Event e, var details, Node target){  
    window.location.href = allEntries.firstWhere((e)=>e.cname == target.id).url;
  }
}
  