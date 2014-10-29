import 'package:polymer/polymer.dart';
import 'package:inputzone/inputzone.dart';
import 'dart:html';

@CustomTag('iz-output')
class IzOutput extends PolymerElement {
    
  @published FileTask context;
  
  IzOutput.created() : super.created(){  
  }
  
  void remove(){
    dispatchEvent(new CustomEvent('remove',detail: this.id ));
  }
  
  void CancelORDownloadClicked(){
    if(STATUSTYPE.values[context.status_id]== STATUSTYPE.COMPLETED){
      var a = new AnchorElement(href: context.output_file_url)..attributes.addAll({'Download':context.output_file_name});
      document.body.children.add(a);
      a.click();
    }
    else
      dispatchEvent(new CustomEvent('cancel',detail: this.id ));
  }
  
}