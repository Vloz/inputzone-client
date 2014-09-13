import 'dart:html';
import 'package:polymer/polymer.dart';
import 'package:template_binding/template_binding.dart';

void main(){
  initPolymer().run(() {
                  Polymer.onReady.then((_) {
                    var template = document.querySelector('#iz-body');
                    templateBind(template).model = new BodyModel();
                  });
                });
  window.navigator.temporaryStorage.requestQuota(1024*1024*1024*2,(_){
    ///TODO:Trouver pourquoi ce callback accepte pas l'utilisation de l'input
    print("Local file enabled!");
    },(e)=>window.alert("Cannot use your storage. Process aborted:"+e.message));
}


class BodyModel extends Observable {
  @observable String value = 'something';
}
