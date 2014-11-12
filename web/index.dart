import 'dart:html';
import 'package:polymer/polymer.dart';
import 'package:inputzone/iz_register.dart';
import 'package:paper_elements/paper_input.dart';


PaperInput searchbox;


void main(){
  initPolymer();
  searchbox = querySelector('#registerSearch');
  searchbox.onInput.listen((e){
    (querySelector('iz-register') as IzRegister).filter(searchbox.inputValue);
    });
}