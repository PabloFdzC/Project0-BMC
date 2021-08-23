#include <gtk/gtk.h>
#include "global.h"
#include "genotypes.h"
#include <math.h>

GtkBuilder *builder; 
GtkWidget *window;
GtkStack *stack;
GtkWidget *switcher;

GtkWidget *btn_loadFile;
GtkWidget *btn_saveFile;

GtkWidget *btn_nFeatures;
GtkWidget *txt_nFeatures;

GtkWidget *txt_dominantLetter;
GtkWidget *txt_dominantTrait;
GtkWidget *txt_recesiveLetter;
GtkWidget *txt_recesiveTrait;
GtkWidget *btn_saveTrait;

GtkWidget *lst_traits;

GtkWidget *grid_mendel;

char tmp[128];

Array_char letters;
Array_chars strList;
Array_chars rbtnNames;
Array_chars headersMendel;


enum Role{FATHER, MOTHER};
typedef Array(enum Role) Array_Role;
Array_Role roles;

Array_char letters;

int done = 0;

void fillGridGenotypes();
void fillParentsCbx();
void on_father_toggled(GtkToggleButton* w);
void on_mother_toggled(GtkToggleButton* w);

int main(int argc, char *argv[]){
  initArray(letters, char, 10);
  initArray(roles, enum Role, 10);
  initArray(rbtnNames, Array_char, 10);
  insertArray(letters, char, 'A');
  insertArray(letters, char, 'B');
  insertArray(letters, char, 'C');
  insertArray(letters, char, 'D');
  insertArray(letters, char, 'E');

  gtk_init(&argc, &argv);

  builder = gtk_builder_new();
  gtk_builder_add_from_file (builder, "glade/window.glade", NULL);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
  gtk_window_maximize(GTK_WINDOW(window));
  gtk_builder_connect_signals(builder, NULL);

  gtk_widget_show(window);
  gtk_main();

  return 0;
}

void on_window_destroy(){
  g_object_unref(builder);
  freeArray(letters);
  freeArray(roles);
  freeArrayP(strList);
  freeArrayP(rbtnNames);
  gtk_main_quit();
}

void on_btn_nFeatures_clicked(GtkButton *b){
  printf("%s\n",tmp);
}

void on_txt_nFeatures_changed(GtkEntry *e){
  sprintf(tmp, "entry:%s\n", gtk_entry_get_text(e));
}

void on_stack_set_focus_child(GtkContainer* container, GtkWidget* child){
  const gchar *c = gtk_stack_get_visible_child_name(stack);
  if(c[0] == '1'){
    fillGridGenotypes();
  } else if(c[0] == '2'){
    fillParentsCbx();
  }
}

void fillParentsCbx(){
  if(strList.used > 0){
    GtkListStore *liststoreF;
    GtkListStore *liststoreM;
    GtkTreeIter iterF;
    GtkTreeIter iterM;
    GtkComboBox *cbx_father;
    GtkComboBox *cbx_mother;

    cbx_father = GTK_COMBO_BOX(gtk_builder_get_object(builder, "cbx_father"));
    cbx_mother = GTK_COMBO_BOX(gtk_builder_get_object(builder, "cbx_mother"));

    liststoreM = gtk_list_store_new(1, G_TYPE_STRING);
    liststoreF = gtk_list_store_new(1, G_TYPE_STRING);

    gtk_combo_box_set_model(GTK_COMBO_BOX(cbx_father), GTK_TREE_MODEL(liststoreF));
    gtk_combo_box_set_model(GTK_COMBO_BOX(cbx_mother), GTK_TREE_MODEL(liststoreM));

    for(int i = 0; i < strList.used; i++){
      if(roles.data[i] == FATHER){
        gtk_list_store_append(GTK_LIST_STORE(liststoreF), &iterF);
        gtk_list_store_set(GTK_LIST_STORE(liststoreF), &iterF, 0, strList.data[i].data, -1);
      } else if(roles.data[i] == MOTHER){
        gtk_list_store_append(GTK_LIST_STORE(liststoreM), &iterM);
        gtk_list_store_set(GTK_LIST_STORE(liststoreM), &iterM, 0, strList.data[i].data, -1);
      }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(cbx_father), 0);
    gtk_combo_box_set_active(GTK_COMBO_BOX(cbx_mother), 0);
  }
}

void fillGridGenotypes(){
  if(!done){
    strList = generateGenotypes(letters);

    deleteAllArray(roles);
    GtkWidget *grid_genotypes;
    int half = strList.used / 2;
    int nameL;
    
    grid_genotypes = GTK_WIDGET(gtk_builder_get_object(builder, "grid_genotypes"));

    for(int row = 0; row < strList.used; row++){
      nameL = snprintf( NULL, 0, "%d", row ) + 1;
      Array_char name;
      initArray(name, char, nameL);
      snprintf(name.data, nameL, "%d", row );
      name.used = nameL;
      name.data[nameL-1] = '\0';
      insertArray(rbtnNames, Array_char, name);

      gtk_grid_insert_row(GTK_GRID(grid_genotypes), row);
      
      GtkWidget *rBtn1 = gtk_radio_button_new_with_label_from_widget(NULL, strList.data[row].data);
      GtkWidget *rBtn2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rBtn1), strList.data[row].data);

      gtk_widget_set_name(GTK_WIDGET(rBtn1), name.data);
      gtk_widget_set_name(GTK_WIDGET(rBtn2), name.data);

      g_signal_connect(rBtn1, "toggled", G_CALLBACK(on_father_toggled), NULL);
      g_signal_connect(rBtn2, "toggled", G_CALLBACK(on_mother_toggled), NULL);

      gtk_grid_attach(GTK_GRID(grid_genotypes), rBtn1, 1, row, 1, 1);
      gtk_grid_attach(GTK_GRID(grid_genotypes), rBtn2, 2, row, 1, 1);
      gtk_widget_show(rBtn1);
      gtk_widget_show(rBtn2);

      if(row > half){
        insertArray(roles, enum Role, MOTHER);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rBtn2), TRUE);
      } else {
        insertArray(roles, enum Role, FATHER);
      }
    }
    
    done = 1;
  }
}

void on_rbtn_identicalGen_toggled(GtkToggleButton* iG){

}

void on_rbtn_identicalFen_toggled(GtkToggleButton* iF){

}

void changeRole(GtkToggleButton* w, enum Role r){
  long n;
  char *name = gtk_widget_get_name(GTK_WIDGET(w));
  n = strtol(name, &name, 10);
  roles.data[n] = r;
}

void on_father_toggled(GtkToggleButton* w){
  if (gtk_toggle_button_get_active(w)){  
    changeRole(w, FATHER);
  }
}

void on_mother_toggled(GtkToggleButton* w){
  if (gtk_toggle_button_get_active(w)){
    changeRole(w, MOTHER);
  }
}

void fillMendelGrid(gchar *fatherTxt, gchar *motherTxt){
  int length = strlen(fatherTxt);
  int maxLet = length/2;
  Array_int pattern;
  initArray(pattern, Array_int, maxLet);
  for(int i = 1; i <= maxLet; i++){
    insertArray(pattern, int, (int)pow(2.0, (double)maxLet-i));
  }
  int total = (int)pow(2.0, (double)maxLet);
  //for(int i = 0; i < total; i++){
    //for(int j = 0; i < total; j++){

    //}
  //}

  freeArray(pattern);
}

void on_btn_mendel_clicked(GtkButton *b){
  GtkComboBox *cbx_father;
  GtkComboBox *cbx_mother;
	gchar *fatherTxt = "";
  gchar *motherTxt = "";

  cbx_father = GTK_COMBO_BOX(gtk_builder_get_object(builder, "cbx_father"));
  cbx_mother = GTK_COMBO_BOX(gtk_builder_get_object(builder, "cbx_mother"));

	fatherTxt = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cbx_father));
  motherTxt = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cbx_mother));
  
  //fillMendelGrid(fatherTxt, motherTxt);
}
//https://www.youtube.com/watch?v=SvEBHBRept8&list=PLmMgHNtOIstZEvqYJncYUx52n8_OV0uWy&index=25
