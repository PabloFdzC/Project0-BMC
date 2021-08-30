#include <gtk/gtk.h>
#include "global.h"
#include <string.h>
#include "genotypes.h"
#include "descendants.h"
#include "quicksort.h"
#include <math.h>

GtkBuilder *builder;
GtkBuilder *builder2;
GtkWidget *window;
GtkWidget *window1;
GtkWidget *loadError;
GtkWidget *GtkBoxMain;
GtkWidget *boxMain;
GtkWidget *view;
GtkWidget *gtkFixed;
GtkStack  *stack;
GtkWidget *switcher;

GtkWidget *btn_loadFile;
GtkWidget *btn_saveFile;

GtkWidget *btn_nFeatures;
GtkWidget *txt_nFeatures;

GtkWidget *txt_dominantLetter;
GtkWidget *txt_dominantTrait;
GtkWidget *txt_recessiveLetter;
GtkWidget *txt_recessiveTrait;
GtkWidget *btn_saveTrait;
GtkWidget *btn_chooseFile;

GtkWidget *lst_traits;

GtkWidget *grid_mendel;
int grid_mendelRows = 0;

int check = 0;
int checkError = 0;
int genesIntro;
int genesFinal;

enum
{
  COL_DOMINANT,
  COL_DOMINANTT,
  COL_RECESIVE,
  COL_RECESIVET,
  NUM_COLS
} ;


GtkListStore  *store;
GtkTreeIter    iter;

char tmp[128];
char file[200];
char fileSave[100];

char dominantL[25];
char dominantT[25];
char recessiveL[25];
char recessiveT[25];

Array_chars strList = {NULL, 0, 0};
Array_chars rbtnNames;

Array_chars headersMendel;
Array_chars gene;
//Array_char  temp;
Array_char domLettersC = {NULL, 0, 0};
Array_chars domTraitsC = {NULL, 0, 0};
Array_chars recTraitsC = {NULL, 0, 0};

Array_chars headersMendelX;
Array_chars headersMendelY;

enum Role{FATHER, MOTHER};
typedef Array(enum Role) Array_Role;
Array_Role roles;

bool doneGenotypes = false;
bool doneParentsCbx = false;
bool done3 = false;
bool isPhenotypeSelected = false;

Array_chars colors;
Array_int colorsCount;
Array_int colorsUsed;
Array_chars percentages;

int lastRow = 0;
int lastCol = 0;

int maxRows = 15;
int maxCols = 8;
typedef Array(GtkCssProvider*) Array_provider;
Array_provider providers;
Array_int providersIndex;

void create_view_and_model();
void fillTraitsTreeView();
void fillGridGenotypes();
void fillParentsCbx();
void on_father_toggled(GtkToggleButton* w);
void on_mother_toggled(GtkToggleButton* w);
void createMendelGrid();
void createMendelHeaders(gchar *fatherTxt, gchar *motherTxt, int maxLet, int total);
void colorMendelGrid();
void createColors();
void createAllDescendants(int total);
void fillMendelGridAux();
void countColors();

int main(int argc, char *argv[]){
  initArray(roles, enum Role, 10);
  initArray(rbtnNames, Array_char, 10);
  initArray(headersMendelX, Array_char, 32);
  initArray(headersMendelY, Array_char, 32);
  initArray(colors, Array_char, 32);
  initArray(providers, GtkCssProvider*, 32);
  initArray(providersIndex, int, 32);
  initArray(colorsCount, int, 32);
  initArray(colorsUsed, int, 32);
  colorsUsed.used = 32;
  colorsCount.used = 32;
  initDescendants(32);

  gtk_init(&argc, &argv);

  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "glade/window.glade", NULL);


  loadError = GTK_WIDGET(gtk_builder_get_object(builder, "loadError"));

  window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
  grid_mendel = GTK_WIDGET(gtk_builder_get_object(builder, "grid_mendel"));
  gtk_window_maximize(GTK_WINDOW(window));
  gtk_builder_connect_signals(builder, NULL);

  create_view_and_model();

  btn_chooseFile = GTK_WIDGET(gtk_builder_get_object(builder,"file1"));

  gtk_widget_show(window);
  createMendelGrid();
  gtk_main();
  return 0;
}

void create_view_and_model(){
  GtkCellRenderer     *renderer;

  view = GTK_WIDGET(gtk_builder_get_object(builder, "tv_traits"));
  store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,G_TYPE_STRING);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (view),-1,
    "Dominant",renderer,"text", COL_DOMINANT,NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),-1,
    "Dominant trait",renderer,"text", COL_DOMINANTT,NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),-1,
    "Recesive",renderer,"text", COL_RECESIVE,NULL);
  
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),-1,
    "Recesive trait",renderer,"text", COL_RECESIVET,NULL);

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL(store));
  gtk_widget_show_all(view);
}

void on_window_destroy(){
  g_object_unref(builder);
  if(domLettersC.size != 0){
    freeArray(domLettersC);
  }
  if(domTraitsC.size != 0){
    freeArrayP(domTraitsC);
  }
  if(recTraitsC.size != 0){
    freeArrayP(recTraitsC);
  }
  freeArray(roles);
  freeArrayP(strList);
  freeArrayP(rbtnNames);
  freeArrayP(headersMendelX);
  freeArrayP(headersMendelY);
  freeArray(colors);
  freeArray(colorsCount);
  freeArray(colorsUsed);
  freeArray(providers);
  freeArray(providersIndex);
  freeDescendants();
  gtk_main_quit();
}

void on_btn_chooseFile_file_set(GtkFileChooserButton *f){
  strcpy(file,gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(f)));
  check = 1;
}


void on_btn_loadFile_clicked(){
  Array_char domLetters;
  Array_chars domTraits;
  Array_chars recTraits;
  if(check == 1){
    doneGenotypes = false;
    doneParentsCbx = false;
    initArray(domLetters, char, 10);
    initArray(domTraits, Array_char, 10);
    initArray(recTraits, Array_char, 10);
    bool dLFilled = false;
    bool dTFilled = false;
    bool badFormat = false;
    char c;
    FILE * flujo = fopen (file ,"rb");
    if (flujo == NULL){
      perror ("Error");
    }

    for(c = fgetc(flujo); feof(flujo) == 0; c = fgetc(flujo)){
      if(badFormat){
        break;
      }
      if(isspace(c)){
        continue;
      }
      if(isalpha(c)){
        if(!dLFilled){
          if(c >= 65 && c <= 90){
            insertArray(domLetters, char, c);
            dLFilled = true;
          } else {
            badFormat = true;
            break;
          }
        } else if(!dTFilled){
          Array_char dT;
          initArray(dT, char, 10);
          while(c != ','){
            if(c == '\n' || feof(flujo)){
              badFormat = true;
              break;
            }
            if(isalpha(c) || (isspace(c) && c != '\n')){
              insertArray(dT, char, c);
            } else {
              badFormat = true;
              break;
            }
            c = fgetc(flujo);
          }
          if(dT.used == 0){
            badFormat = true;
          } else {
            insertArray(dT, char, '\0');
            insertArray(domTraits, Array_char, dT);
            dTFilled = true;
          }
        } else {
          Array_char rT;
          initArray(rT, char, 10);
          while(c != '\n'){
            if(feof(flujo)){
              break;
            }
            if(isalpha(c) || isspace(c)){
              insertArray(rT, char, c);
            } else {
              badFormat = true;
              break;
            }
            c = fgetc(flujo);
          }
          if(rT.used == 0){
            badFormat = true;
            break;
          } else {
            insertArray(rT, char, '\0');
            insertArray(recTraits, Array_char, rT);
            dLFilled = false;
            dTFilled = false;
          }
        }
      }
    }
    fclose(flujo);
    if(badFormat){
      freeArrayP(domTraits);
      freeArrayP(recTraits);
      freeArray(domLetters);
    } else {
      domLettersC = domLetters;
      domTraitsC = domTraits;
      recTraitsC = recTraits;
      fillTraitsTreeView();
    }
  }
}

void fillTraitsTreeView(){
  gtk_list_store_clear(store);
  for(int i = 0; i < domLettersC.used; i++){
    char d[8];
    d[0] = domLettersC.data[i];
    d[1] = '\0';
    char r[8];
    r[0] = domLettersC.data[i]+32;
    r[1] = '\0';
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,COL_DOMINANT, d,COL_DOMINANTT, domTraitsC.data[i].data,COL_RECESIVE,r,COL_RECESIVET,recTraitsC.data[i].data,-1);
  }
}

void on_txt_file_changed(GtkEntry *e){
  *fileSave = '\0';
  strcat(fileSave,gtk_entry_get_text(e));
}


void on_btn_saveFile_clicked(){

}

void on_btn_saveFile(char Pgen[25],char PdominantT[25],char PrecessiveT[25]){

    FILE* fichero;
    fichero = fopen(fileSave, "a+");
    fputs(Pgen, fichero);
    fputs(",",fichero);
    fputs(PdominantT, fichero);
    fputs(",",fichero);
    fputs(PrecessiveT, fichero);
    fputs(",\n",fichero);
    fclose(fichero);
    printf("Proceso completado");

}


void on_btn_nFeatures_clicked(GtkButton *b){
  genesIntro = tmp[0] - '0';
  checkError = 1;
}

void on_txt_nFeatures_changed(GtkEntry *e){
  *tmp = '\0';
  strcat(tmp,gtk_entry_get_text(e));
}

void on_txt_dominantLetter_changed(GtkEntry *e){
   *dominantL = '\0';
   strcat(dominantL,gtk_entry_get_text(e));

}

void on_txt_dominantTrait_changed(GtkEntry *e){
   *dominantT = '\0';
   strcat(dominantT,gtk_entry_get_text(e));
}

void on_txt_recessiveLetter_changed(GtkEntry *e){
   *recessiveL = '\0';
   strcat(recessiveL,gtk_entry_get_text(e));

}

void on_txt_recessiveTrait_changed(GtkEntry *e){
   *recessiveT = '\0';
   strcat(recessiveT,gtk_entry_get_text(e));
}

void on_btn_saveTrait_clicked(){

    if(checkError == 1){
        gtk_list_store_append (store, &iter);

      
        gtk_list_store_set (store, &iter,
                    COL_DOMINANT, dominantL,
                    COL_DOMINANTT, dominantT,
                    COL_RECESIVE,recessiveT,
                    -1);
        insertArray(domLettersC, char, dominantL[0]);
        genesFinal += 1;
        on_btn_saveFile(dominantL,dominantT,recessiveT);

        if (genesFinal == genesIntro){
            fillGridGenotypes();
        }
    }

    if(checkError == 0){
      gtk_widget_show_all (loadError);
    }


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
  if(!doneParentsCbx){
    doneParentsCbx = true;
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
  if(!doneGenotypes){
    doneGenotypes = true;
    if(strList.used > 0){
      freeArrayP(strList);
    }
    strList = generateGenotypes(domLettersC);

    deleteAllArray(roles);
    GtkWidget *grid_genotypes;
    int half = strList.used / 2;
    int nameL;
    
    grid_genotypes = GTK_WIDGET(gtk_builder_get_object(builder, "grid_genotypes"));
    int row = 0;
    for(; row < strList.used; row++){
      nameL = snprintf( NULL, 0, "%d", row ) + 1;
      Array_char name;
      initArray(name, char, nameL);
      snprintf(name.data, nameL, "%d", row );
      name.used = nameL;
      name.data[nameL-1] = '\0';
      insertArray(rbtnNames, Array_char, name);
      GtkWidget *rBtn1 = gtk_grid_get_child_at(GTK_GRID(grid_genotypes), 1, row);
      GtkWidget *rBtn2 = gtk_grid_get_child_at(GTK_GRID(grid_genotypes), 2, row);
      if(rBtn1){
        gtk_button_set_label(GTK_BUTTON(rBtn1), strList.data[row].data);
        gtk_button_set_label(GTK_BUTTON(rBtn2), strList.data[row].data);
        gtk_widget_set_visible(GTK_WIDGET(rBtn1), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(rBtn2), TRUE);
      }else{
        gtk_grid_insert_row(GTK_GRID(grid_genotypes), row);
        rBtn1 = gtk_radio_button_new_with_label_from_widget(NULL, strList.data[row].data);
        rBtn2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rBtn1), strList.data[row].data);
        gtk_widget_set_name(GTK_WIDGET(rBtn1), name.data);
        gtk_widget_set_name(GTK_WIDGET(rBtn2), name.data);
        gtk_grid_attach(GTK_GRID(grid_genotypes), rBtn1, 1, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid_genotypes), rBtn2, 2, row, 1, 1);
      }

      gtk_widget_set_tooltip_text(rBtn1, strList.data[row].data); // Creates tooltip
      gtk_widget_set_tooltip_text(rBtn2, strList.data[row].data); // Creates tooltip
      

      g_signal_connect(rBtn1, "toggled", G_CALLBACK(on_father_toggled), NULL);
      g_signal_connect(rBtn2, "toggled", G_CALLBACK(on_mother_toggled), NULL);

      gtk_widget_show(rBtn1);
      gtk_widget_show(rBtn2);

      if(row > half){
        insertArray(roles, enum Role, MOTHER);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rBtn2), TRUE);
      } else {
        insertArray(roles, enum Role, FATHER);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rBtn1), TRUE);
      }
    }
    while(true){
      GtkWidget *rBtn1 = gtk_grid_get_child_at(GTK_GRID(grid_genotypes), 1, row);
      GtkWidget *rBtn2 = gtk_grid_get_child_at(GTK_GRID(grid_genotypes), 2, row);
      if(rBtn1){
        gtk_widget_set_visible(GTK_WIDGET(rBtn1), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(rBtn2), FALSE);
        row++;
      } else {
        break;
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
  doneParentsCbx = false;
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

  int maxLet = strlen(fatherTxt)/2;
  int total = (int)pow(2.0, (double)maxLet);
  createMendelHeaders(fatherTxt, motherTxt, maxLet, total);
  createAllDescendants(total);
  createColors();
  fillMendelGridAux();
}

void fillMendelGridAux(){
  int lRow = lastRow;
  int lCol = lastCol;
  int r=0,c=0;
  printf("EMPIEZA\n");
  for(int row = 0; row <= maxRows; row++){
    lRow = row + lastRow;
    if(lastRow == 0){
      r = row-1;
    } else {
      r = row+lastRow-maxRows-1;
    }
    for(int col = 0; col <= maxCols; col++){
      GtkWidget* cell = gtk_grid_get_child_at(GTK_GRID(grid_mendel), col, row);
      lCol = col + lastCol;
      if(lastCol == 0){
        c = col-1;
      }else{
        c = col+lastCol-maxCols-1;
      }
      if(row == 0 && col > 0){
        printf("r:%d, c:%d, lCol:%d\n", r,c, lCol);
        if(lCol-1 < headersMendelX.used){
          gtk_frame_set_label(GTK_FRAME(cell), headersMendelX.data[lCol-1].data);
        } else {
          printf("SIII\n");
          gtk_frame_set_label(GTK_FRAME(cell), "");
        }
      } else if(col == 0 && row > 0){
        printf("r:%d, c:%d, lRow:%d\n", r,c, lRow);
        if(lRow-1 < headersMendelY.used){
          gtk_frame_set_label(GTK_FRAME(cell), headersMendelY.data[lRow-1].data);
        } else {
          gtk_frame_set_label(GTK_FRAME(cell), "");
        }
      } else if(row > 0 && col > 0){
        printf("r:%d, c:%d\n", r,c);
        Array_char d = getDescendant(r,c);
        if(d.data == NULL){
          gtk_frame_set_label(GTK_FRAME(cell), "");
        } else {
          gtk_frame_set_label(GTK_FRAME(cell), d.data);
        }
      }
      gtk_frame_set_shadow_type(GTK_FRAME(cell), GTK_SHADOW_NONE);
      gtk_widget_set_vexpand(cell, TRUE);
    }
  }
  lastRow = lRow;
  lastCol = lCol;
  colorMendelGrid();
}

void createAllDescendants(int total){
  if(hasDescendants()){
    freeDescendants();
    initDescendants(total);
  }
  grid_mendelRows = total;
  for(int row = 0; row <total; row++){
    for(int col = 0; col < total; col++){
      createDescendant(headersMendelX.data[col], headersMendelY.data[row], row);
    }
  }
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
  lastRow = 0;
  lastCol = 0;
  fillMendelGrid(fatherTxt, motherTxt);
}

void createMendelGrid(){
  for(int i = 0; i <= maxRows; i++){
    gtk_grid_insert_row(GTK_GRID(grid_mendel), i);
  }
  for(int i = 0; i <= maxCols; i++){
    gtk_grid_insert_column(GTK_GRID(grid_mendel), i);
  }
  for(int row = 0; row <= maxRows; row++){
    for(int col = 0; col <= maxCols; col++){
      GtkWidget* cell = gtk_grid_get_child_at(GTK_GRID(grid_mendel), col, row);
      if(!cell){
        cell = gtk_frame_new("");
        gtk_frame_set_label_align(GTK_FRAME(cell), 0.5, 0.5);
        gtk_grid_attach(GTK_GRID(grid_mendel), cell, col, row, 1, 1);
        gtk_widget_show(cell);
      }
    }
  }
  gtk_grid_set_row_spacing(GTK_GRID(grid_mendel), 10);
  gtk_grid_set_column_spacing(GTK_GRID(grid_mendel), 10);
}

void createMendelHeaders(gchar *fatherTxt, gchar *motherTxt, int maxLet, int total){
  int reuseX = 0;
  int reuseY = 0;
  if(headersMendelX.used > 0){
    reuseX = headersMendelX.used;
  }
  if(headersMendelY.used > 0){
    reuseY = headersMendelY.used;
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
    if(i < reuseX){
      headerX = headersMendelX.data[i];
      deleteAllArray(headerX);
    } else {
      initArray(headerX, char, maxLet+1);
    }
    if(i < reuseY){
      headerY = headersMendelY.data[i];
      deleteAllArray(headerY);
    } else {
      initArray(headerY, char, maxLet+1);
    }
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
    if(i >= reuseX){
      insertArray(headersMendelX, Array_char, headerX);
    }else{
      headersMendelX.data[i] = headerX;
    }
    if(i >= reuseY){
      insertArray(headersMendelY, Array_char, headerY);
    }else{
      headersMendelY.data[i] = headerY;
    }
  }

  freeArray(pattern);
  freeArray(patternMax);
  freeArray(patternIndex);
}

void colorMendelGrid(){
  int colorIndex = 0;
  for(int i = 0; i < colorsUsed.used; i++){
    colorsUsed.data[i] = -1;
    colorsCount.data[i] = -1;
  }
  int r = 0;
  int c = 0;
  int cont = 0;
  for(int row = 1; row <= maxRows; row++){
    for(int col = 1; col <= maxCols; col++){
      GtkWidget* cell = gtk_grid_get_child_at(GTK_GRID(grid_mendel), col, row);
      r = row+lastRow-maxRows-1;
      c = col+lastCol-maxCols-1;
      Array_char d = getDescendant(r,c);
      GtkStyleContext * context = gtk_widget_get_style_context(cell);
      if(providersIndex.used > cont){
        gtk_style_context_remove_provider(context, GTK_STYLE_PROVIDER(providers.data[providersIndex.data[cont]]));
      }
      if(d.data == NULL){
        gtk_frame_set_label(GTK_FRAME(cell), "");
      } else {
        colorIndex = getColorIndex(d, isPhenotypeSelected);
        gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(providers.data[colorIndex]),GTK_STYLE_PROVIDER_PRIORITY_USER);
        if(providersIndex.used == cont){
          insertArray(providersIndex, int, colorIndex);
        } else {
          providersIndex.data[cont] = colorIndex;
        }
      }
      cont++;
    }
  }
}

void createColors(){
  int totColors = getMaxColor();
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
      GtkCssProvider *provider;
      provider = gtk_css_provider_new();
      gtk_css_provider_load_from_data(provider, colors.data[i-1].data,-1,NULL);
      insertArray(providers, GtkCssProvider*, provider);

    }
  } 
}

void countColors(){
  int colorIndex = 0;
  for(int i = 0; i < colorsUsed.used; i++){
    colorsUsed.data[i] = -1;
    colorsCount.data[i] = -1;
  }
  for(int r = 0; r < grid_mendelRows; r++){
    for(int c = 0; c < grid_mendelRows;c++){
      Array_char d = getDescendant(r,c);
      colorIndex = getColorIndex(d, isPhenotypeSelected);
      if(colorsCount.data[colorIndex] == -1){
        colorsCount.data[colorIndex] = 1;
      } else {
        colorsCount.data[colorIndex]++;
      }
      colorsUsed.data[colorIndex] = colorIndex;
    }
  }
  quicksort(colorsCount, colorsUsed);
}

void fillPercentages(){
  if(!done3){
    countColors();
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

void on_btn_up_clicked(GtkButton *b){
  if(lastRow - 2*maxRows >= 0){
    lastRow = lastRow - 2*maxRows;
    lastCol = lastCol - maxCols;
    fillMendelGridAux();
  }
}

void on_btn_left_clicked(GtkButton *b){
  if(lastCol - 2*maxCols >= 0){
    lastRow = lastRow - maxRows;
    lastCol = lastCol - 2*maxCols;
    fillMendelGridAux();
  }
}

void on_btn_down_clicked(GtkButton *b){
  if(headersMendelY.used - lastRow > 0){//NOSE
    lastCol = lastCol - maxCols;
    //lastRow = lastRow + maxRows;
    fillMendelGridAux();
  }
}

void on_btn_right_clicked(GtkButton *b){
  if(headersMendelX.used - lastCol > 0){//NOSE
    lastRow = lastRow - maxRows;
    //lastCol = lastCol + maxCols;
    fillMendelGridAux();
  }
}

//https://www.youtube.com/watch?v=SvEBHBRept8&list=PLmMgHNtOIstZEvqYJncYUx52n8_OV0uWy&index=25


