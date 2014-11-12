part of InputZone;

class RegisterEntry extends Observable{
  @observable String cname;
  @observable String url;
  @observable bool pnacl;
  @observable bool emscr;
  @observable String desc;
  @observable ObservableList<String> intag;
  @observable ObservableList<String> outag;
  
  RegisterEntry.fromJSON(Map json){
    cname = json['cname'].toUpperCase();
    url = json['url'];
    pnacl = json['pnacl'];
    emscr = json['emscr'];
    intag = json['intag'];
    outag = json['outag'];
    desc = json['desc'];
  }

}