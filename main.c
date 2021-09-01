#include <gtk/gtk.h>
#include "global.h"
#include <string.h>
#include "genotypes.h"
#include "descendants.h"
#include "quicksort.h"
#include <math.h>

GtkBuilder *builder;
GtkBuilder *builder2;
GtkBuilder *builder3;
GtkWidget *window;
GtkWidget *window1;
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

bool check = false;

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

Array_char domLettersC;
Array_chars domTraitsC;
Array_chars recTraitsC;

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
GtkWidget *lbl_descendantsPages;
int desActualPageX = 1;
int desActualPageY = 1;
int desTotPageX = 1;
int desTotPageY = 1;

int maxParentsShow = 32;
GtkWidget *lbl_parentsPages;
int lastParent = 0;
int parActualPage = 1;
int parTotPage = 1;

int maxColorsShow = 5;
GtkWidget *lbl_percentagePages;
int lastColorP = 0;
Array_provider providersPe;
Array_int providersIndexPe;
int perActualPage = 1;
int perTotPage = 1;

Array_chars tooltips;
Array_char tooltip;

bool blockOther = false;

bool tvsReady = false;
GtkTreeIter iterTVS;

GtkWidget *txt_fileName;

int indexTV = -1;
bool traitSaved = false;

bool mendelFilled = false;

GtkTreeSelection* tvs_selection;

void create_view_and_model();
void fillTraitsTreeView();
void fillGridGenotypes();
void fillGridGenotypesAux();
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
void tooltipArray(Array_char parent);
void setEntriesRestrictions();
void insertLetterInTreeView(char c, char domT[], char recT[]);
void messagesWindow(char m[]);

int main(int argc, char *argv[]){
  initArray(roles, enum Role, 10);
  initArray(domLettersC, char, 26);
  initArrayP(domTraitsC, Array_char, 26);
  initArrayP(recTraitsC, Array_char, 26);
  initArrayP(rbtnNames, Array_char, 10);
  initArrayP(headersMendelX, Array_char, 32);
  initArrayP(headersMendelY, Array_char, 32);
  initArrayP(colors, Array_char, 32);
  initArray(providers, GtkCssProvider*, 32);
  initArray(providersIndex, int, 32);
  initArray(colorsCount, int, 32);
  initArray(colorsUsed, int, 32);
  initArray(tooltip, char, 32);
  colorsUsed.used = 32;
  colorsCount.used = 32;
  initDescendants(32);

  gtk_init(&argc, &argv);

  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "glade/window.glade", NULL);

  window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
  grid_mendel = GTK_WIDGET(gtk_builder_get_object(builder, "grid_mendel"));
  lbl_descendantsPages = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_descendantsPages"));
  txt_fileName = GTK_WIDGET(gtk_builder_get_object(builder, "txt_fileName"));
  gtk_window_maximize(GTK_WINDOW(window));
  gtk_builder_connect_signals(builder, NULL);

  create_view_and_model();

  btn_chooseFile = GTK_WIDGET(gtk_builder_get_object(builder,"file1"));

  setEntriesRestrictions();

  gtk_widget_show(window);
  createMendelGrid();
  gtk_main();
  return 0;
}

void setEntriesRestrictions(){
  txt_dominantLetter = GTK_WIDGET(gtk_builder_get_object(builder, "txt_dominantLetter"));
  txt_recessiveLetter = GTK_WIDGET(gtk_builder_get_object(builder, "txt_recessiveLetter"));
  txt_dominantTrait = GTK_WIDGET(gtk_builder_get_object(builder, "txt_dominantTrait"));
  txt_recessiveTrait = GTK_WIDGET(gtk_builder_get_object(builder, "txt_recessiveTrait"));
  txt_nFeatures = GTK_WIDGET(gtk_builder_get_object(builder, "txt_nFeatures"));
  gtk_entry_set_max_length (GTK_ENTRY(txt_dominantLetter), 1);
  gtk_entry_set_max_length (GTK_ENTRY(txt_recessiveLetter), 1);
  gtk_entry_set_max_length (GTK_ENTRY(txt_nFeatures), 1);
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
  tvs_selection = GTK_TREE_SELECTION(gtk_builder_get_object(builder, "tvs_selection"));
  gtk_widget_show_all(view);
  tvsReady = true;
}

void on_window_destroy(){
  g_object_unref(builder);
  if(domLettersC.data){
    freeArray(domLettersC);
  }
  if(domTraitsC.data){
    freeArrayP(domTraitsC);
  }
  if(recTraitsC.data){
    freeArrayP(recTraitsC);
  }
  freeArray(roles);
  freeArrayP(strList);
  freeArrayP(rbtnNames);
  freeArrayP(headersMendelX);
  freeArrayP(headersMendelY);
  freeArrayP(colors);
  freeArray(colorsCount);
  freeArray(colorsUsed);
  freeArray(providers);
  freeArray(providersIndex);
  freeArray(tooltip);
  freeDescendants();
  gtk_main_quit();
}

void on_btn_chooseFile_file_set(GtkFileChooserButton *f){
  strcpy(file,gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(f)));
  check = true;
}

void on_btn_loadFile_clicked(){
  if(check){
    check = false;
    Array_char domLetters;
    Array_chars domTraits;
    Array_chars recTraits;
    doneGenotypes = false;
    doneParentsCbx = false;
    initArray(domLetters, char, 25);
    initArrayP(domTraits, Array_char, 25);
    initArrayP(recTraits, Array_char, 25);
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
            for(int i = 0; i < domLetters.used; i++){
              if(domLetters.data[i] == c){
                badFormat = true;
                break;
              }
            }
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
      messagesWindow("File format is wrong");
    } else if(domLetters.used > 7){
      freeArrayP(domTraits);
      freeArrayP(recTraits);
      freeArray(domLetters);
      messagesWindow("More than 7 traits are not supported");
    } else {
      if(domLettersC.size > 0){
        freeArray(domLettersC);
      }
      if(domTraitsC.size > 0){
        freeArrayP(domTraitsC);
      }
      if(recTraitsC.size > 0){
        freeArrayP(recTraitsC);
      }
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
    insertLetterInTreeView(domLettersC.data[i], domTraitsC.data[i].data, recTraitsC.data[i].data);
  }
}

void on_btn_saveFile_clicked(){
  const gchar* fn = gtk_entry_get_text(GTK_ENTRY(txt_fileName));
  bool isAlphaN = isalnum(fn[0]);
  bool isGood = domLettersC.used > 0 && domLettersC.used == domTraitsC.used && domTraitsC.used == recTraitsC.used;
  if(isAlphaN){
    gchar* fileName = g_strconcat(fn, ".txt", NULL);
    if(isGood){
      FILE* f;
      f = fopen(fileName, "w");
      for(int i = 0; i < domLettersC.used; i++){
        fputc(domLettersC.data[i], f);
        fputc(',', f);
        fputs(domTraitsC.data[i].data, f);
        fputc(',', f);
        fputs(recTraitsC.data[i].data, f);
        if(i != domLettersC.used-1){
          fputc('\n', f);
        }
      }
      fclose(f);
    }
    g_free(fileName);
  }
  if(!isAlphaN && !isGood){
    messagesWindow("Bad name for file and all traits need to be filled");
  } else if(!isAlphaN){
    messagesWindow("Bad name for file");
  } else if(!isGood){
    messagesWindow("All traits need to be filled");
  } else {
    messagesWindow("File saved");
  }
}

void on_txt_fileName_insert_text(GtkEditable* self, gchar* new_text, gint new_text_length, int position, gpointer user_data){
  if(!isalnum(new_text[new_text_length-1]) && new_text[new_text_length-1] != 32 && new_text[new_text_length-1] != '-' && new_text[new_text_length-1] != '_'){
    g_signal_stop_emission_by_name (G_OBJECT(self), "insert_text");
  }
}

void on_btn_nFeatures_clicked(GtkButton *b){
  doneParentsCbx = false;
  doneGenotypes = false;
  const gchar* val = gtk_entry_get_text(GTK_ENTRY(txt_nFeatures));
  int n = 0;
  bool isIn;
  if(gtk_entry_get_text_length(GTK_ENTRY(txt_nFeatures)) > 0){
    sscanf(val, "%d", &n);
  }
  if(n > 0){
    if(domLettersC.used == 0){
      for(char c = 'A'; c <= 'Z'; c++){
        if(n > 0){
          insertArray(domLettersC, char, c);
          insertLetterInTreeView(c, "", "");
        } else {
          break;
        }
        n--;
      }
    } else if(n > domLettersC.used){
      n = n - domLettersC.used;
      for(char c = 'A'; c <= 'Z'; c++){
        if(n > 0){
          isIn = false;
          for(int i = 0; i < domLettersC.used; i++){
            if(c == domLettersC.data[i]){
              isIn = true;
              break;
            }
          }
          if(!isIn){
            insertArray(domLettersC, char, c);
            insertLetterInTreeView(c, "", "");
            n--;
          }
        } else {
          break;
        }
      }
    } else if(n < domLettersC.used) {
      GtkTreePath *path;
      for(; n < domLettersC.used; domLettersC.used--){
        path = gtk_tree_path_new_from_indices(domLettersC.used-1, -1);
        if(gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path)){
          gtk_list_store_remove(store, &iter);
        }
      }
      gtk_tree_path_free(path);
      for(int i = n; i < domTraitsC.size; i++){
        if(domTraitsC.data[i].data){
          freeArray(domTraitsC.data[i]);
          Array_char empty = {NULL, 0, 0};
          domTraitsC.data[i] = empty;
          domTraitsC.used--;
        }
      }
      for(int i = n; i < recTraitsC.size; i++){
        if(recTraitsC.data[i].data){
          freeArray(recTraitsC.data[i]);
          Array_char empty = {NULL, 0, 0};
          recTraitsC.data[i] = empty;
          recTraitsC.used--;
        }
      }
    }
  } else {
    messagesWindow("Number of traits cannot be empty");
  }
}


void on_btn_edit_clicked(GtkButton *b){
  if(tvsReady){
    traitSaved = false;
    gchar *value;
    gchar *value2;
    GtkTreeModel *model;
    gtk_tree_selection_get_selected(tvs_selection,&model, &iterTVS);
    gtk_tree_model_get(model, &iterTVS, COL_DOMINANT, &value,COL_RECESIVE, &value2, -1);
    for(int i = 0; i < domLettersC.used; i++){
      if(value[0] == domLettersC.data[i]){
        indexTV = i;
      }
    }
    if(indexTV != -1){
      gtk_entry_set_text(GTK_ENTRY(txt_dominantLetter), value);
      if(indexTV < domTraitsC.used){
        if(domTraitsC.data[indexTV].data){
          gtk_entry_set_text(GTK_ENTRY(txt_dominantTrait), domTraitsC.data[indexTV].data);
        } else {
          gtk_entry_set_text(GTK_ENTRY(txt_dominantTrait), "");
        }
      } else {
        gtk_entry_set_text(GTK_ENTRY(txt_dominantTrait), "");
      }
      if(indexTV < recTraitsC.used){
        if(recTraitsC.data[indexTV].data){
          gtk_entry_set_text(GTK_ENTRY(txt_recessiveTrait), recTraitsC.data[indexTV].data);
        } else {
          gtk_entry_set_text(GTK_ENTRY(txt_recessiveTrait), "");
        }
      } else {
        gtk_entry_set_text(GTK_ENTRY(txt_recessiveTrait), "");
      }
    }
  }
}

void insertLetterInTreeView(char c, char domT[], char recT[]){
  char d[8];
  d[0] = c;
  d[1] = '\0';
  char r[8];
  r[0] = c+32;
  r[1] = '\0';
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter,COL_DOMINANT, d,COL_DOMINANTT, domT,COL_RECESIVE,r,COL_RECESIVET,recT,-1);
}

void on_txt_traits_insert_text(GtkEditable* self, gchar* new_text, gint new_text_length, int position, gpointer user_data){
  traitSaved = false;
  if(!isalpha(new_text[new_text_length-1]) && new_text[new_text_length-1] != 32){
    g_signal_stop_emission_by_name (G_OBJECT(self), "insert_text");
  }
}

void on_txt_nFeatures_insert_text(GtkEditable* self, gchar* new_text, gint new_text_length, int position, gpointer user_data){
  if(new_text[new_text_length-1] < '1' || new_text[new_text_length-1] > '7'){
    g_signal_stop_emission_by_name (G_OBJECT(self), "insert_text");
  }
}

void on_txt_dominantLetter_insert_text(GtkEditable* self, gchar* new_text, gint new_text_length, int position, gpointer user_data){
  if(new_text[new_text_length-1] < 'A' || new_text[new_text_length-1] > 'Z'){
    g_signal_stop_emission_by_name (G_OBJECT(self), "insert_text");
  } else {
    if(!blockOther){
      blockOther = true;
      char c[2];
      c[0] = new_text[new_text_length-1] + 32;
      c[1] = '\0';
      gtk_entry_set_text(GTK_ENTRY(txt_recessiveLetter), c);
    } else {
      blockOther = false;
    }
  }
}

void on_txt_recessiveLetter_insert_text(GtkEditable* self, gchar* new_text, gint new_text_length, int position, gpointer user_data){
  if(new_text[new_text_length-1] < 'a' || new_text[new_text_length-1] > 'z'){
    g_signal_stop_emission_by_name (G_OBJECT(self), "insert_text");
  } else {
    if(!blockOther){
      blockOther = true;
      char c[2];
      c[0] = new_text[new_text_length-1] - 32;
      c[1] = '\0';
      gtk_entry_set_text(GTK_ENTRY(txt_dominantLetter), c);
    } else {
      blockOther = false;
    }
  }
}

void on_btn_saveTrait_clicked(GtkButton *b){
  if(!traitSaved){
    doneGenotypes = false;
    traitSaved = true;
    if(indexTV > -1){
      if(indexTV < domLettersC.used){
        int dL = gtk_entry_get_text_length(GTK_ENTRY(txt_dominantLetter));
        int dTL = gtk_entry_get_text_length(GTK_ENTRY(txt_dominantTrait));
        int rL = gtk_entry_get_text_length(GTK_ENTRY(txt_recessiveLetter));
        int rTL = gtk_entry_get_text_length(GTK_ENTRY(txt_recessiveTrait));
        if(dL > 0 && dTL > 0 && rL > 0 && rTL > 0){
          bool inDLC = false;
          const gchar* d = gtk_entry_get_text(GTK_ENTRY(txt_dominantLetter));
          const gchar* dT = gtk_entry_get_text(GTK_ENTRY(txt_dominantTrait));
          const gchar* r = gtk_entry_get_text(GTK_ENTRY(txt_recessiveLetter));
          const gchar* rT = gtk_entry_get_text(GTK_ENTRY(txt_recessiveTrait));

          for(int i = 0; i < domLettersC.used; i++){
            if(i != indexTV && d[0] == domLettersC.data[i]){
              inDLC = true;
              break;
            }
          }
          if(inDLC){
            messagesWindow("Letter is already in list of traits");
          } else {
            Array_char dTNew;
            initArray(dTNew, char, dTL+1);
            for(int i = 0; i < dTL; i++){
              insertArray(dTNew, char, dT[i]);
            }
            if(dTNew.data[dTNew.used-1] != '\0'){
              insertArray(dTNew, char, '\0');
            }
            Array_char rTNew;
            initArray(rTNew, char, rTL+1);
            for(int i = 0; i < rTL; i++){
              insertArray(rTNew, char, rT[i]);
            }
            if(rTNew.data[rTNew.used-1] != '\0'){
              insertArray(rTNew, char, '\0');
            }
            if(domTraitsC.data[indexTV].data){
              freeArray(domTraitsC.data[indexTV]);
              domTraitsC.used--;
            }
            if(recTraitsC.data[indexTV].data){
              freeArray(recTraitsC.data[indexTV]);
              recTraitsC.used--;
            }
            domTraitsC.data[indexTV] = dTNew;
            domTraitsC.used++;
            recTraitsC.data[indexTV] = rTNew;
            recTraitsC.used++;
            domLettersC.data[indexTV] = d[0];
            gtk_list_store_set(store, &iterTVS,COL_DOMINANT, d,COL_DOMINANTT, dT,COL_RECESIVE,r,COL_RECESIVET,rT,-1);
          }
          
        } else {
          messagesWindow("All entries must be filled");
        }
      }
    } else {
      messagesWindow("No trait is selected");
    }
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
  if(!doneParentsCbx && doneGenotypes){
    lastParent = 0;
    parActualPage = 1;
    parTotPage = 1;
    doneParentsCbx = true;
    doneGenotypes = false;
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
  if(!doneGenotypes && domLettersC.used > 0 && domLettersC.used == domTraitsC.used && domTraitsC.used == recTraitsC.used){
    lastParent = 0;
    parActualPage = 1;
    parTotPage = 1;
    doneGenotypes = true;
    if(strList.used > 0){
      freeArrayP(strList);
    }
    strList = generateGenotypes(domLettersC);

    deleteAllArray(roles);

    int half = strList.used / 2;
    GtkWidget *grid_genotypes;
    grid_genotypes = GTK_WIDGET(gtk_builder_get_object(builder, "grid_genotypes"));

    for(int row = 0; row < strList.used; row++){
      if(row > half){
        insertArray(roles, enum Role, MOTHER);
      } else {
        insertArray(roles, enum Role, FATHER);
      }
    }

    for(int row = 0; row < maxParentsShow; row++){
      GtkWidget *rBtn1 = gtk_grid_get_child_at(GTK_GRID(grid_genotypes), 1, row);
      GtkWidget *rBtn2 = gtk_grid_get_child_at(GTK_GRID(grid_genotypes), 2, row);
      if(rBtn1){
        gtk_widget_set_visible(GTK_WIDGET(rBtn1), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(rBtn2), TRUE);
      } else {
        gtk_grid_insert_row(GTK_GRID(grid_genotypes), row);
        rBtn1 = gtk_radio_button_new_from_widget(NULL);
        rBtn2 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(rBtn1));
        gtk_grid_attach(GTK_GRID(grid_genotypes), rBtn1, 1, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid_genotypes), rBtn2, 2, row, 1, 1);
      }

      g_signal_connect(rBtn1, "toggled", G_CALLBACK(on_father_toggled), NULL);
      g_signal_connect(rBtn2, "toggled", G_CALLBACK(on_mother_toggled), NULL);

      gtk_widget_show(rBtn1);
      gtk_widget_show(rBtn2);

    }

    fillGridGenotypesAux();
  }
}

void fillGridGenotypesAux(){
  int nameL;
  int row = 0;

  GtkWidget *grid_genotypes;
  grid_genotypes = GTK_WIDGET(gtk_builder_get_object(builder, "grid_genotypes"));

  for(; row < maxParentsShow; row++){
    nameL = snprintf(NULL, 0, "%d", lastParent) + 1;
    Array_char name;
    initArray(name, char, nameL);
    snprintf(name.data, nameL, "%d", lastParent);
    name.used = nameL;
    name.data[nameL-1] = '\0';
    insertArray(rbtnNames, Array_char, name);
    GtkWidget *rBtn1 = gtk_grid_get_child_at(GTK_GRID(grid_genotypes), 1, row);
    GtkWidget *rBtn2 = gtk_grid_get_child_at(GTK_GRID(grid_genotypes), 2, row);
    if(rBtn1){
      if(lastParent < roles.used){
        gtk_widget_set_visible(GTK_WIDGET(rBtn1), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(rBtn2), TRUE);
        gtk_button_set_label(GTK_BUTTON(rBtn1), strList.data[lastParent].data);
        gtk_button_set_label(GTK_BUTTON(rBtn2), strList.data[lastParent].data);
        tooltipArray(strList.data[lastParent]);
        gtk_widget_set_tooltip_text(rBtn1, tooltip.data); // Creates tooltip
        gtk_widget_set_tooltip_text(rBtn2, tooltip.data); // Creates tooltip
        if(roles.data[lastParent] == MOTHER){
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rBtn2), TRUE);
        } else {
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rBtn1), TRUE);
        }
      } else {
        gtk_widget_set_visible(GTK_WIDGET(rBtn1), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(rBtn2), FALSE);
      }
    }  
    lastParent++;
  }
  if(strList.used % maxParentsShow == 0){
    parTotPage = strList.used / maxParentsShow;
  } else{
    parTotPage = strList.used / maxParentsShow + 1;
  }
  char lbl[14];
  lbl_parentsPages = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_parentsPages"));
  sprintf(lbl, "%d of %d", parActualPage, parTotPage);
  gtk_label_set_text(GTK_LABEL(lbl_parentsPages), lbl);

}

void tooltipArray(Array_char parent){
  deleteAllArray(tooltip);
  bool dominant;
  int current;
  for(int i = 0; i < parent.used-1; i+=2){
    dominant = (parent.data[i] >= 'A' && parent.data[i] <= 'Z') || (parent.data[i+1] >= 'A' && parent.data[i+1] <= 'Z');
    current = i/2;
    for(int j = 0; j < domTraitsC.data[current].used-1; j++){
      if(dominant){
        insertArray(tooltip, char, domTraitsC.data[current].data[j]);
      } else {
        insertArray(tooltip, char, recTraitsC.data[current].data[j]);
      }
    }
    if(i != parent.used-3){
      insertArray(tooltip, char, ',');
    }
  }
  insertArray(tooltip, char, '\0');
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
  mendelFilled = false;
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
  if(!mendelFilled){
    mendelFilled = true;
    int maxLet = strlen(fatherTxt)/2;
    int total = (int)pow(2.0, (double)maxLet);
    if(headersMendelX.used > 0){
      freeArrayP(headersMendelX);
      freeArrayP(headersMendelY);
      initArrayP(headersMendelX, Array_char, total);
      initArrayP(headersMendelY, Array_char, total);
    }
    createMendelHeaders(fatherTxt, motherTxt, maxLet, total);
    createAllDescendants(total);
    createColors();
    fillMendelGridAux();
  }
}

void fillMendelGridAux(){
  int lRow = lastRow;
  int lCol = lastCol;
  int r=0,c=0;
  for(int row = 0; row <= maxRows; row++){
    lRow = row + lastRow;
    r = row+lastRow-1;
    for(int col = 0; col <= maxCols; col++){
      GtkWidget* cell = gtk_grid_get_child_at(GTK_GRID(grid_mendel), col, row);
      lCol = col + lastCol;
      c = col+lastCol-1;
      if(row == 0 && col > 0){
        if(lCol-1 < headersMendelX.used){
          gtk_frame_set_label(GTK_FRAME(cell), headersMendelX.data[lCol-1].data);
        } else {
          gtk_frame_set_label(GTK_FRAME(cell), "");
        }
      } else if(col == 0 && row > 0){
        if(lRow-1 < headersMendelY.used){
          gtk_frame_set_label(GTK_FRAME(cell), headersMendelY.data[lRow-1].data);
        } else {
          gtk_frame_set_label(GTK_FRAME(cell), "");
        }
      } else if(row > 0 && col > 0){
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
  char lbl[23];
  if(grid_mendelRows % maxRows == 0){
    desTotPageY = grid_mendelRows / maxRows;
  } else{
    desTotPageX = grid_mendelRows / maxRows + 1;
  }
  if(grid_mendelRows % maxCols == 0){
    desTotPageY = grid_mendelRows / maxCols;
  } else{
    desTotPageY = grid_mendelRows / maxCols + 1;
  }
  sprintf(lbl, "(%d,%d) of (%d,%d)", desActualPageX, desActualPageY, desTotPageX, desTotPageY);
  gtk_label_set_text(GTK_LABEL(lbl_descendantsPages), lbl);
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
  desActualPageX = 1;
  desActualPageY = 1;
  desTotPageX = 1;
  desTotPageY = 1;
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

void on_trait_changed(){
  traitSaved = false;
}

void on_parent_changed(){
  mendelFilled = false;
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
        colorIndex = getColorIndex(d, isPhenotypeSelected, NULL);
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
    if(colors.used < i){
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
  int totalDes = -1;
  deleteAllArray(colorsUsed);
  deleteAllArray(colorsCount);

  for(int i = 0; i < getMaxColor(); i++){
    insertArray(colorsUsed, int, -1);
    insertArray(colorsCount, int, -1);
  }
  for(int r = 0; r < grid_mendelRows; r++){
    for(int c = 0; c < grid_mendelRows;c++){
      Array_char d = getDescendant(r,c);
      colorIndex = getColorIndex(d, isPhenotypeSelected, &totalDes);
      if(colorsCount.data[colorIndex] == -1){
        colorsCount.data[colorIndex] = totalDes;
      } else {
        colorsCount.data[colorIndex]+=totalDes;
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
    bool onlyRemove = false;
    double p;
    double sum = 0.0;
    GtkWidget *percentages_box = GTK_WIDGET(gtk_builder_get_object(builder2, "percentages_box"));
    for(int i = 0; i < colorsCount.used; i++){
      if(colorsUsed.data[i] == -1){
        colorsUsed.used = i;
        colorsCount.used = i;
        break;
      }
      sum += colorsCount.data[i];
    }
    initArrayP(percentages, Array_char, colorsCount.used);
    for(int i = 0; i < maxColorsShow; i++){
      char percStr[10] = "";
      if(lastColorP < colorsUsed.used){
        if(colorsUsed.data[lastColorP] != -1){
          p = ((double)colorsCount.data[lastColorP]) / sum * 100;
          sprintf(percStr, "%.2f %%", p);
        }
      } else {
        onlyRemove = true;
      }
      GtkWidget* cell = gtk_grid_get_child_at(GTK_GRID(percentages_box), i, 1);
      if(!cell){
        gtk_grid_insert_column(GTK_GRID(percentages_box), i);
        cell = gtk_frame_new("");
        gtk_frame_set_label_align(GTK_FRAME(cell), 0.5, 0.5);
        gtk_grid_attach(GTK_GRID(percentages_box), cell, i, 1, 1, 1);
      }
      gtk_frame_set_label(GTK_FRAME(cell), percStr);
      gtk_frame_set_shadow_type(GTK_FRAME(cell), GTK_SHADOW_NONE);
      gtk_widget_set_vexpand(cell, TRUE);
      gtk_widget_set_margin_top(cell, ((int)(100.0-p)));
      gtk_widget_show(cell);
      
      GtkCssProvider *provider;
      GtkStyleContext * context = gtk_widget_get_style_context(cell);   
      if(providersPe.used == i){
        if(lastColorP < colorsUsed.used){
          provider = gtk_css_provider_new();
          gtk_css_provider_load_from_data(provider, colors.data[colorsUsed.data[lastColorP]].data,-1,NULL);
          insertArray(providersPe, GtkCssProvider*, provider);
          gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),GTK_STYLE_PROVIDER_PRIORITY_USER);
        }
      } else {
        if(onlyRemove){
          if(i < providersPe.used){
            if(providersPe.data[i]){
              gtk_style_context_remove_provider(context, GTK_STYLE_PROVIDER(providersPe.data[i]));
              g_object_unref(providersPe.data[i]);
            }
            providersPe.data[i] = NULL;
          }
        } else {
          if(i < providersPe.used){
            if(providersPe.data[i]){
              gtk_style_context_remove_provider(context, GTK_STYLE_PROVIDER(providersPe.data[i]));
              g_object_unref(providersPe.data[i]);
            }
            if(lastColorP < colorsUsed.used){
              provider = gtk_css_provider_new();
              gtk_css_provider_load_from_data(provider, colors.data[colorsUsed.data[lastColorP]].data,-1,NULL);
              providersPe.data[i] = provider;
              gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),GTK_STYLE_PROVIDER_PRIORITY_USER);
            }
          }
        }
        onlyRemove = false;
      }
      lastColorP++;
    }
    if(colorsUsed.used % maxColorsShow == 0){
      perTotPage = colorsUsed.used / maxColorsShow;
    } else{
      perTotPage = colorsUsed.used / maxColorsShow + 1;
    }
    char lbl[14];
    lbl_percentagePages = GTK_WIDGET(gtk_builder_get_object(builder2, "lbl_percentagePages"));
    sprintf(lbl, "%d of %d", perActualPage, perTotPage);
    gtk_label_set_text(GTK_LABEL(lbl_percentagePages), lbl);
  }
}

void percentagesWindow(){
  initArray(providersPe, GtkCssProvider*, colorsCount.used);
  initArray(providersIndexPe, int, colorsCount.used);
  lastColorP = 0;

  builder2 = gtk_builder_new();
  gtk_builder_add_from_file(builder2, "glade/percentage.glade", NULL);

  GtkWidget *percentage_window = GTK_WIDGET(gtk_builder_get_object(builder2, "percentage_window"));
  gtk_window_resize (GTK_WINDOW(percentage_window),350, 200);
  gtk_window_set_transient_for(GTK_WINDOW(percentage_window), GTK_WINDOW(window));
  gtk_window_set_modal(GTK_WINDOW(percentage_window), TRUE);
  gtk_builder_connect_signals(builder2, NULL);

  lbl_percentagePages = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_percentagePages"));
  gtk_widget_show(percentage_window);
  gtk_main();
}

void on_percentage_window_close(){
  if(done3){
    done3 = false;
    freeArrayP(percentages);
    freeArray(providersPe);
    freeArray(providersIndexPe);
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
    desActualPageX--;
    fillMendelGridAux();
  }
}

void on_btn_left_clicked(GtkButton *b){
  if(lastCol - 2*maxCols >= 0){
    lastRow = lastRow - maxRows;
    lastCol = lastCol - 2*maxCols;
    desActualPageY--;
    fillMendelGridAux();
  }
}

void on_btn_down_clicked(GtkButton *b){
  if(headersMendelY.used - lastRow > 0){
    lastCol = lastCol - maxCols;
    desActualPageX++;
    fillMendelGridAux();
  }
}

void on_btn_right_clicked(GtkButton *b){
  if(headersMendelX.used - lastCol > 0){
    lastRow = lastRow - maxRows;
    desActualPageY++;
    fillMendelGridAux();
  }
}

void on_btn_nextPe_clicked(GtkButton *b){
  if(lastColorP < colorsCount.used){
    done3 = false;
    perActualPage++;
    fillPercentages();
  }
}

void on_btn_previousPe_clicked(GtkButton *b){
  if(lastColorP - 2*maxColorsShow >= 0){
    done3 = false;
    lastColorP = lastColorP - 2*maxColorsShow;
    perActualPage--;
    fillPercentages();
  }
}

void on_btn_previousPa_clicked(GtkButton *b){
  if(lastParent - 2*maxParentsShow >= 0){
    lastParent = lastParent - 2*maxParentsShow;
    parActualPage--;
    fillGridGenotypesAux();
  }
}

void on_btn_nextPa_clicked(GtkButton *b){
  if(lastParent < roles.used){
    parActualPage++;
    fillGridGenotypesAux();
  }
}

void messagesWindow(char m[]){
  builder3 = gtk_builder_new();
  gtk_builder_add_from_file(builder3, "glade/messages.glade", NULL);

  GtkWidget *messages_window = GTK_WIDGET(gtk_builder_get_object(builder3, "messages_window"));
  gtk_window_resize (GTK_WINDOW(messages_window),350, 200);
  gtk_window_set_transient_for(GTK_WINDOW(messages_window), GTK_WINDOW(window));
  gtk_window_set_modal(GTK_WINDOW(messages_window), TRUE);
  gtk_builder_connect_signals(builder3, NULL);

  GtkWidget *lbl_message = GTK_WIDGET(gtk_builder_get_object(builder3, "lbl_message"));
  gtk_label_set_text(GTK_LABEL(lbl_message), m);  
  gtk_widget_show(messages_window);
  gtk_main();  
}

void on_messages_window_destroy(){
  g_object_unref(builder3);
  gtk_main_quit();
}

//https://www.youtube.com/watch?v=SvEBHBRept8&list=PLmMgHNtOIstZEvqYJncYUx52n8_OV0uWy&index=25
