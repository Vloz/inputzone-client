part of InputZone;

class RegisterEntry extends Observable{
  @observable String cname;
  @observable String url;
  @observable bool pnacl;
  @observable bool emscr;
  @observable String desc;
  @observable ObservableList<String> intag = new ObservableList<String>();
  @observable ObservableList<String> outag = new ObservableList<String>();
  
  RegisterEntry.fromJSON(Map json){
    cname = json['cname'].toUpperCase();
    url = json['url'];
    pnacl = json['pnacl'];
    emscr = json['emscr'];
    intag.addAll(json['intag']);
    outag.addAll(json['outag']);
    desc = json['desc'];
  }

}