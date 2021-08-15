#include <gtk/gtk.h>
#include "global.h"
#include "genotypes.h"

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

GtkWidget *grid_genotypes;

char tmp[128];

Array_char letters;
Array_chars strList;
int done = 0;

void fillGrid();

int main(int argc, char *argv[]){
  initArray(letters, char, 10);
  insertArray(letters, char, 'A');
  insertArray(letters, char, 'B');

  gtk_init(&argc, &argv);

  builder = gtk_builder_new();
  gtk_builder_add_from_file (builder, "glade/window.glade", NULL);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  btn_nFeatures = GTK_WIDGET(gtk_builder_get_object(builder, "btn_nFeatures"));
  txt_nFeatures = GTK_WIDGET(gtk_builder_get_object(builder, "txt_nFeatures"));
  switcher = GTK_WIDGET(gtk_builder_get_object(builder, "switcher"));
  stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
  grid_genotypes = GTK_WIDGET(gtk_builder_get_object(builder, "grid_genotypes"));
  gtk_window_maximize(GTK_WINDOW(window));
  gtk_builder_connect_signals(builder, NULL);

  g_object_unref(builder);

  gtk_widget_show(window);
  gtk_main();

  return 0;
}

void on_window_destroy(){
  freeArray(letters);
  freeArrayP(strList);
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
    fillGrid();
  }
}

void fillGrid(){
  if(!done){
    strList = generateGenotypes(letters);
    for(int row = 0; row < strList.used; row++){
      gtk_grid_insert_row(GTK_GRID(grid_genotypes), row);
      GtkWidget *rBtn1 = gtk_radio_button_new_with_label_from_widget(NULL, strList.data[row].data);
      GtkWidget *rBtn2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rBtn1), strList.data[row].data);

      gtk_grid_attach(GTK_GRID(grid_genotypes), rBtn1, 1, row, 1, 1);
      gtk_grid_attach(GTK_GRID(grid_genotypes), rBtn2, 2, row, 1, 1);
      gtk_widget_show(rBtn1);
      gtk_widget_show(rBtn2);
    }
    done = 1;
  }
}
//https://www.youtube.com/watch?v=SvEBHBRept8&list=PLmMgHNtOIstZEvqYJncYUx52n8_OV0uWy&index=25