import 'package:polymer/polymer.dart';
import 'package:inputzone/inputzone.dart';
import 'dart:html';

@CustomTag('iz-output')
class IzOutput extends PolymerElement {

//  @published String filename="Error! Filename wasnt loaded...";
//  @published int progress=0;
//  @published int preprogress=0;
//  @published String status="unknown";
    
  @published FileTask context;
  
  IzOutput.created() : super.created(){  
  }
  
  void remove(){
    dispatchEvent(new CustomEvent('remove',detail: this.id ));
  }
  
  void CancelORDownloadClicked(){
    if(STATUSTYPE.values[context.status_id]== STATUSTYPE.COMPLETED)
    new AnchorElement(href: context.output_file_url)..attributes.addAll({'Download':''})..click();
    
   // window.location.assign(context.output_file_url);
  }
  
}