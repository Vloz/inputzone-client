import 'package:polymer/polymer.dart';
import 'dart:html';
import 'dart:js';
import 'inputzone.dart';
import 'package:paper_elements/paper_toast.dart';


@CustomTag('iz-converter')
class IzConverter extends PolymerElement {



  
  IzConverter.created() : super.created(){ 
    if(this.attributes.containsKey('active'))
      initConverter();
  }
  
  void ready(){
//    var importDoc = document.currentScript.ownerDocument;
//    //var t = importDoc.querySelector('#disqus');
//    var clone = document.importNode(this.shadowRoot.querySelector('#disqus'), true);
//    //document.body.append(clone);
//    this.shadowRoot.append(clone);
//    
//    var dsq = document.createElement('script'); 
//    dsq.type = 'text/javascript'; 
//    dsq.async = true;
//    dsq.src = '//' + 'inputzone' + '.disqus.com/embed.js';
//    document.getElementsByTagName('body')[0].append(dsq);
  }
 
  
}


