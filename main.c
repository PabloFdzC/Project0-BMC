#include <gtk/gtk.h>
#include "global.h"
#include "genotypes.h"
#include "descendants.h"
#include "quicksort.h"
#include <math.h>
//#include <ncurses.h>

GtkBuilder *builder;
GtkBuilder *builder2; 
GtkWidget *window;
GtkStack  *stack;
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
GtkWidget *btn_chooseFile;

GtkWidget *lst_traits;

GtkWidget *grid_mendel;
int grid_mendelRows = 0;

char tmp[128];
char file[200];

Array_chars strList;
Array_chars rbtnNames;

Array_chars headersMendel;
Array_chars gene;
Array_char  temp;
Array_char alpha;

Array_chars headersMendelX;
Array_chars headersMendelY;

enum Role{FATHER, MOTHER};
typedef Array(enum Role) Array_Role;
Array_Role roles;

bool done = false;
bool done2 = false;
bool done3 = false;
int gridDone = 0;
bool isPhenotypeSelected = false;

Array_chars colors;
Array_int colorsCount;
Array_int colorsUsed;
Array_chars percentages;


void fillGridGenotypes();
void fillParentsCbx();
void on_father_toggled(GtkToggleButton* w);
void on_mother_toggled(GtkToggleButton* w);
void createMendelGrid(int total);
void createMendelHeaders(gchar *fatherTxt, gchar *motherTxt, int maxLet, int total);
void colorMendelGrid();
void createColors();

int main(int argc, char *argv[]){

  //initArray(gene, Array_char, 100);

  initArray(temp, char, 100);

  initArray(alpha, char, 10);
  initArray(roles, enum Role, 10);
  initArray(rbtnNames, Array_char, 10);
  initArray(headersMendelX, Array_char, 32);
  initArray(headersMendelY, Array_char, 32);
  initArray(colors, Array_char, 32);
  initArray(colorsCount, int, 32);
  initArray(colorsUsed, int, 32);
  colorsUsed.used = 32;
  colorsCount.used = 32;
  initDescendants();
  
  insertArray(alpha, char, 'A');
  insertArray(alpha, char, 'B');
  insertArray(alpha, char, 'C');
  insertArray(alpha, char, 'D');
  insertArray(alpha, char, 'E');
  insertArray(alpha, char, 'F');
  // insertArray(alpha, char, 'G');
  // insertArray(alpha, char, 'H');
  // insertArray(alpha, char, 'I');
  // insertArray(alpha, char, 'J');


  gtk_init(&argc, &argv);

  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "glade/window.glade", NULL);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
  gtk_window_maximize(GTK_WINDOW(window));
  gtk_builder_connect_signals(builder, NULL);

  btn_chooseFile = GTK_WIDGET(gtk_builder_get_object(builder,"file1"));

  gtk_widget_show(window);
  gtk_main();

  return 0;
}

void on_window_destroy(){
  g_object_unref(builder);
  freeArray(alpha);
  freeArray(temp);
  freeArray(roles);
  freeArrayP(strList);
  freeArrayP(rbtnNames);
  freeArrayP(headersMendelX);
  freeArrayP(headersMendelY);
  freeArray(colors);
  freeArray(colorsCount);
  freeArray(colorsUsed);
  freeDescendants();
  gtk_main_quit();
}

void on_btn_chooseFile_file_set(GtkFileChooserButton *f){

  strcpy(file,gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(f)));
  printf("file name = %s\n",gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(f)));
}


void on_btn_loadFile_clicked(){

  char caracter;
  FILE * flujo = fopen (file ,"rb");
  if (flujo == NULL){
    perror ("Error");

  }


  while (feof(flujo) == 0){

    caracter = fgetc(flujo);


    if(temp.used < 1){
      insertArray(temp,char,caracter);
    }
    if (caracter == 10){
      caracter = fgetc(flujo);
      if (caracter >= 65 && caracter <= 122){
          insertArray(temp,char,caracter);
      }

    }

    printf("%c", caracter );

  }

  for (int i = 0; i<temp.used;i++){
    insertArray(alpha, char, temp.data[i]);
    printf("%c",temp.data[i]);
  }

  fillGridGenotypes();
  fillParentsCbx();
  fclose(flujo);

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
  if(!done2){
    done2 = true;
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
}

void fillGridGenotypes(){
  if(!done){
    done = true;
    strList = generateGenotypes(alpha);

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
    
  }
}

void on_rbtn_identicalGen_toggled(GtkToggleButton* iG){
  if (gtk_toggle_button_get_active(iG)){  
    isPhenotypeSelected = false;
    colorMendelGrid();
  }
}

void on_rbtn_identicalPhen_toggled(GtkToggleButton* iF){
  if (gtk_toggle_button_get_active(iF)){  
    isPhenotypeSelected = true;
    colorMendelGrid();
  }
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
  grid_mendel = GTK_WIDGET(gtk_builder_get_object(builder, "grid_mendel"));

  int maxLet = strlen(fatherTxt)/2;
  int total = (int)pow(2.0, (double)maxLet);

  if(grid_mendelRows != total){
    if(grid_mendelRows != 0){
      gtk_widget_destroy(grid_mendel);
      grid_mendel = gtk_grid_new();
      GtkWidget *vport_mendel = GTK_WIDGET(gtk_builder_get_object(builder, "vport_mendel"));
      gtk_container_add(GTK_CONTAINER(vport_mendel), grid_mendel);
    }
    gtk_grid_set_row_spacing(GTK_GRID(grid_mendel), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid_mendel), 10);
    createMendelGrid(total);
  }
  createMendelHeaders(fatherTxt, motherTxt, maxLet, total);
  for(int row = 0; row <= total; row++){
    for(int col = 0; col <= total; col++){
      GtkWidget* cell = gtk_grid_get_child_at(GTK_GRID(grid_mendel), col, row);
      if(row == 0 && col > 0){
        gtk_frame_set_label(GTK_FRAME(cell), headersMendelX.data[col-1].data);
      } else if(col == 0 && row > 0){
        gtk_frame_set_label(GTK_FRAME(cell), headersMendelY.data[row-1].data);
      } else if(row > 0 && col > 0){
        Array_char d = createDescendant(headersMendelX.data[col-1], headersMendelY.data[row-1]);
        gtk_frame_set_label(GTK_FRAME(cell), d.data);
      }
      gtk_frame_set_shadow_type(GTK_FRAME(cell), GTK_SHADOW_NONE);
      gtk_widget_set_vexpand(cell, TRUE);
    }
  }
  colorMendelGrid();
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
  
  fillMendelGrid(fatherTxt, motherTxt);
}

void createMendelGrid(int total){
  if(hasDescendants()){
    freeDescendants();
    initDescendants();
  }
  grid_mendelRows = total;
  for(int i = 0; i < total; i++){
    gtk_grid_insert_row(GTK_GRID(grid_mendel), i);
    gtk_grid_insert_column(GTK_GRID(grid_mendel), i);
  }

  for(int row = 0; row <= total; row++){
    for(int col = 0; col <= total; col++){
      GtkWidget* cell = gtk_frame_new("");
      gtk_frame_set_label_align (GTK_FRAME(cell), 0.5, 0.5);
      gtk_grid_attach(GTK_GRID(grid_mendel), cell, col, row, 1, 1);
      gtk_widget_show(cell);
    }
  }
}

void createMendelHeaders(gchar *fatherTxt, gchar *motherTxt, int maxLet, int total){
  if(headersMendelX.used > 0){
    freeArrayP(headersMendelX);
    initArray(headersMendelX, Array_char, total);
  }
  if(headersMendelY.used > 0){
    freeArrayP(headersMendelY);
    initArray(headersMendelY, Array_char, total);
  }
  Array_int pattern;
  Array_int patternMax;
  Array_int patternIndex;
  initArray(pattern, int, maxLet);
  initArray(patternMax, int, maxLet);
  initArray(patternIndex, int, maxLet);
  for(int i = 1; i <= maxLet; i++){
    insertArray(pattern, int, (int)pow(2.0, (double)maxLet-i));
    insertArray(patternMax, int, pattern.data[i-1]);
    insertArray(patternIndex, int, i*2-2);
  }

  
  for(int i = 0; i < total; i++){
    Array_char headerX;
    Array_char headerY;
    initArray(headerX, char, maxLet+1);
    initArray(headerY, char, maxLet+1);
    for(int j = 0; j < maxLet; j++){
      insertArray(headerX, char, fatherTxt[patternIndex.data[j]]);
      insertArray(headerY, char, motherTxt[patternIndex.data[j]]);

      pattern.data[j]--;

      if(pattern.data[j] == 0){
        if(patternIndex.data[j] % 2 == 0){
          patternIndex.data[j]++;
        } else {
          patternIndex.data[j]--;
        }
      }
      
      if(pattern.data[j] == 0){
        pattern.data[j] = patternMax.data[j];
      }
    }
    insertArray(headerX, char, '\0');
    insertArray(headerY, char, '\0');
    insertArray(headersMendelX, Array_char, headerX);
    insertArray(headersMendelY, Array_char, headerY);
  }

  freeArray(pattern);
  freeArray(patternMax);
  freeArray(patternIndex);
}

void colorMendelGrid(){
  createColors();

  int cont = 0;
  int colorIndex = 0;
  for(int i = 0; i < colorsUsed.used; i++){
    colorsUsed.data[i] = -1;
    colorsCount.data[i] = -1;
  }

  for(int row = 1; row <= grid_mendelRows; row++){
    for(int col = 1; col <= grid_mendelRows; col++){
      GtkCssProvider *provider;
      provider = gtk_css_provider_new();
      GtkWidget* cell = gtk_grid_get_child_at(GTK_GRID(grid_mendel), col, row);
      Array_char d = getDescendant(cont);
      colorIndex = getColorIndex(d, isPhenotypeSelected);
      if(colorsCount.data[colorIndex] == -1){
        colorsCount.data[colorIndex] = 1;
      } else {
        colorsCount.data[colorIndex]++;
      }
      colorsUsed.data[colorIndex] = colorIndex;
      gtk_css_provider_load_from_data(provider, colors.data[colorIndex].data,-1,NULL); 
      GtkStyleContext * context = gtk_widget_get_style_context(cell);   
      gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),GTK_STYLE_PROVIDER_PRIORITY_USER);
      cont++;
      g_object_unref (provider);
    }
  }
  quicksort(colorsCount, colorsUsed);
}

void createColors(){
  int totColors = getMaxColor();
  if(colorsUsed.size < totColors){
    freeArray(colorsUsed);
    freeArray(colorsCount);
    initArray(colorsUsed, int, totColors);
    initArray(colorsCount, int, totColors);
  }
  srand ( time(NULL) ); 
  int red = (rand() % (255 - 60 + 1)) + 60;
  int green = (rand() % (255 - 60 + 1)) + 60;
  int blue = (rand() % (255 - 60 + 1)) + 60;
  int r, sum;
  char redC[4], greenC[4], blueC[4];
  for(int i = 0; i <= totColors; i++){
    Array_char color;
    r = (rand() % (255 - 60 + 1)) + 60;
    if(i % 3 == 0){
      red = r;
    } if(i % 2 == 0){
      green = r;
    } else {
      blue = r;
    }
    if(i < colors.used){
      color = colors.data[i];
    } else {
      initArray(color, char, 64);
    }
    snprintf(redC, 4, "%d", red);
    snprintf(greenC, 4, "%d", green);
    snprintf(blueC, 4, "%d", blue);
    sum = (red * 299 + green * 587 + blue * 114) / 1000;
    if(sum > 128){
      sprintf(color.data, "*{background-color: rgb(%s,%s,%s);color:#000;padding:2px;}", redC, greenC, blueC);
    } else{
      sprintf(color.data, "*{background-color: rgb(%s,%s,%s);color:#FFF;padding:2px;}", redC, greenC, blueC);
    }
    color.data[63] = '\0';
    color.used = 64;
    //printf("%s\n", color.data);
    if(colors.used < i){
      insertArray(colors, Array_char, color);
    }
  } 
}

void fillPercentages(){
  if(!done3){
    done3 = true;
    double p;
    double sum = 0.0;
    int size;
    GtkWidget *percentages_box = GTK_WIDGET(gtk_builder_get_object(builder2, "percentages_box"));
    for(int i = 0; i < colorsCount.used; i++){
      if(colorsUsed.data[i] == -1){
        break;
      }
      sum += colorsCount.data[i];
    }
    initArray(percentages, Array_char, colorsCount.used);
    for(int i = 0; i < colorsUsed.used; i++){
      Array_char percStr;
      if(colorsUsed.data[i] == -1){
        break;
      }
      p = ((double)colorsCount.data[i]) / sum * 100;
      size = snprintf(NULL, 0, "%.2f", p) + 2;
      initArray(percStr, Array_char, size);
      snprintf(percStr.data, size, "%.2f", p);
      percStr.used = size;
      percStr.data[size-2] = '%';
      percStr.data[size-1] = '\0';
      GtkWidget* cell = gtk_frame_new("");
      gtk_frame_set_label_align (GTK_FRAME(cell), 0.5, 0.5);
      gtk_frame_set_label(GTK_FRAME(cell), percStr.data);
      gtk_frame_set_shadow_type(GTK_FRAME(cell), GTK_SHADOW_NONE);
      gtk_widget_set_vexpand(cell, TRUE);

      gtk_box_pack_end(GTK_BOX(percentages_box), cell, TRUE, TRUE, 2);
      gtk_widget_show(cell);
      
      GtkCssProvider *provider;
      provider = gtk_css_provider_new();
      
      gtk_css_provider_load_from_data(provider, colors.data[colorsUsed.data[i]].data,-1,NULL); 
      GtkStyleContext * context = gtk_widget_get_style_context(cell);   
      gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),GTK_STYLE_PROVIDER_PRIORITY_USER);
      g_object_unref (provider);
    }
  }
}

void percentagesWindow(){
  builder2 = gtk_builder_new();
  gtk_builder_add_from_file(builder2, "glade/percentage.glade", NULL);

  GtkWidget *percentage_window = GTK_WIDGET(gtk_builder_get_object(builder2, "percentage_window"));
  gtk_window_resize (GTK_WINDOW(percentage_window),200, 200);
  gtk_window_set_transient_for(GTK_WINDOW(percentage_window), GTK_WINDOW(window));
  gtk_window_set_modal(GTK_WINDOW(percentage_window), TRUE);
  gtk_builder_connect_signals(builder2, NULL);

  gtk_widget_show(percentage_window);
  gtk_main();
}

void on_percentage_window_close(){
  if(done3){
    done3 = false;
    freeArrayP(percentages);
    g_object_unref(builder2);
    gtk_main_quit();
  }
}

void on_btn_show_percentages_clicked(GtkButton *b){
  percentagesWindow();
}

void on_percentage_window_focus(GtkWidget* self, GtkDirectionType direction, gpointer user_data){
  fillPercentages();
}

//https://www.youtube.com/watch?v=SvEBHBRept8&list=PLmMgHNtOIstZEvqYJncYUx52n8_OV0uWy&index=25

