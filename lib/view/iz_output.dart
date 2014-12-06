import 'package:polymer/polymer.dart';
import 'package:inputzone/inputzone.dart';
import 'dart:html';

import 'package:inputzone/view/iz_params_base.dart';

@CustomTag('iz-output')
class IzOutput extends PolymerElement {
    
  @published FileTask context;
  @observable bool hasParams = false;
  bool paramsDrawerOpen = false;
  IzParamsBase params=null;
  
  IzOutput.created() : super.created(){  
  }
  
  bool _ready=false;
  void ready(){
    if(_ready)
      return;
    _ready=true;
    if(iz_app.iz_params_content != null){
      hasParams = true;
      if(iz_app.iz_params_content is DivElement){
        params = new Element.tag('iz-params');
              iz_app.iz_params_content.children.forEach((c){
                params.children.add(c.clone(true));
              });
              
      }else{
        params = iz_app.iz_params_content.clone(true);
      }
      params.context = context;
      ($['paramsdrawer'] as DivElement).children.add(params);
    }
  }
  
  void remove(){
    dispatchEvent(new CustomEvent('remove',detail: this.id ));
  }
  
  void DownloadClicked(){
    if(STATUSTYPE.values[context.status_id]== STATUSTYPE.COMPLETED)
      iz_app.downloadTaskOutput(this.context);
      
  }
  
  void ResumeORCancelORDeleteClicked(){
    int s = STATUSTYPE.values[context.status_id];
    if( s == STATUSTYPE.WAITINGUSERCLICK){
      iz_app.querySpaceAndRun(context);
    }
    else if( s == STATUSTYPE.COMPLETED || s == STATUSTYPE.ERRRORED || s == STATUSTYPE.CANCELED || s == STATUSTYPE.WAITINGQUOTA){
     context.deleteFsContent();
     context.remove();
    }else
      context.cancel();
  }
  
  void SettingsClicked(){
    ($['paramsdrawer'] as DivElement).style..height=(paramsDrawerOpen?"0":"auto")
        ..display= (paramsDrawerOpen?"none":"block");
    paramsDrawerOpen = !paramsDrawerOpen;
  }
  
}