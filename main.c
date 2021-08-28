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
GtkWidget *windowTable;
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
GtkWidget *txt_recesiveLetter;
GtkWidget *txt_recesiveTrait;
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
  COL_GEN,
  COL_DOMINANT,
  COL_RECESIVE,
  NUM_COLS
} ;


  GtkListStore  *store;
  GtkTreeIter    iter;


GtkTreeModel *
create_and_fill_model (void)
{

  
  store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  return GTK_TREE_MODEL (store);
}

GtkWidget *
create_view_and_model (void)
{
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view;

  view = gtk_tree_view_new ();

  /* --- Column #1 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "GEN",  
                                               renderer,
                                               "text", COL_GEN,
                                               NULL);

  /* --- Column #2 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "DOMINANT",  
                                               renderer,
                                               "text", COL_DOMINANT,
                                               NULL);

    /* --- Column #3 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "RECESIVE",  
                                               renderer,
                                               "text", COL_RECESIVE,
                                               NULL);



  model = create_and_fill_model ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  return view;
}

char tmp[128];
char file[200];
char fileSave[100];

char dominantL[25];
char dominantT[25];
char recesiveL[25];
char recesiveT[25];

Array_chars strList = {NULL, 0, 0};
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

bool doneGenotypes = false;
bool doneParentsCbx = false;
bool done3 = false;
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
  
  //insertArray(alpha, char, 'A');
  //insertArray(alpha, char, 'B');
  //insertArray(alpha, char, 'C');
  //insertArray(alpha, char, 'D');
  //insertArray(alpha, char, 'E');
  //insertArray(alpha, char, 'F');
  // insertArray(alpha, char, 'G');
  // insertArray(alpha, char, 'H');
  // insertArray(alpha, char, 'I');
  // insertArray(alpha, char, 'J');


  gtk_init(&argc, &argv);

  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "glade/window.glade", NULL);

  windowTable = GTK_WIDGET(gtk_builder_get_object(builder, "windowTable"));
  view = create_view_and_model ();
  gtk_container_add (GTK_CONTAINER (windowTable), view);

  loadError = GTK_WIDGET(gtk_builder_get_object(builder, "loadError"));

  window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
  //gtk_window_maximize(GTK_WINDOW(window));
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
  check = 1;
}


void on_btn_loadFile_clicked(){

  if(check == 1){
      doneGenotypes = false;
      doneParentsCbx = false;
      deleteAllArray(temp);
      deleteAllArray(alpha);
      int check1 = 1;
      int check2 = 0;
      int check3 = 0;
      char caracter;
      char gen[25];
      char dominant[25];
      char recesive[25];
      FILE * flujo = fopen (file ,"rb");
      if (flujo == NULL){
        perror ("Error");

      }

      *gen = '\0';
      *dominant = '\0';
      *recesive = '\0';

      while (feof(flujo) == 0){

        caracter = fgetc(flujo);

        if(temp.used < 1){
          insertArray(temp,char,caracter);
        }


        if(check1 == 1 ){
          strncat(gen, &caracter,1);
          caracter = fgetc(flujo);
          check1 = 0;
        }

        if (caracter == 44 && check2 == 0){
            caracter = fgetc(flujo);
            while(caracter != 44){
              strncat(dominant, &caracter,1);
              printf("%s\n",dominant);
              caracter = fgetc(flujo);
            }
            check3 = 1;
            check2 = 1;
        }

        if (caracter == 44 && check3 == 1){
            caracter = fgetc(flujo);
            while(caracter != 44){
              strncat(recesive, &caracter,1);
              printf("%s\n",dominant);
              caracter = fgetc(flujo);
            }
            check2 = 0;
            check3 = 0;


        }


        if (caracter == 10){

          caracter = fgetc(flujo);

          if (caracter >= 65 && caracter <= 122){
              insertArray(temp,char,caracter);
              strncat(gen, &caracter,1);
              gtk_list_store_append (store, &iter);
              gtk_list_store_set (store, &iter,
                          COL_GEN, gen,
                          COL_DOMINANT, dominant,
                          COL_RECESIVE,recesive,
                          -1);
              *gen = '\0';
              *dominant = '\0';
              *recesive = '\0';
            }

          }


    }

    for (int i = 0; i<temp.used;i++){
      insertArray(alpha, char, temp.data[i]);
    }

    fillGridGenotypes();
    fillParentsCbx();
    gtk_widget_show_all (windowTable);
    fclose(flujo);
  }

  if(check == 0){

    gtk_widget_show_all (loadError);


  }

}

void on_txt_file_changed(GtkEntry *e){
  *fileSave = '\0';
  strcat(fileSave,gtk_entry_get_text(e));
}


void on_btn_saveFile_clicked(){
}

void on_btn_saveFile(char Pgen[25],char PdominantT[25],char PrecesiveT[25]){

    FILE* fichero;
    fichero = fopen(fileSave, "a+");
    fputs(Pgen, fichero);
    fputs(",",fichero);
    fputs(PdominantT, fichero);
    fputs(",",fichero);
    fputs(PrecesiveT, fichero);
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

void on_txt_recesiveLetter_changed(GtkEntry *e){
   *recesiveL = '\0';
   strcat(recesiveL,gtk_entry_get_text(e));

}

void on_txt_recesiveTrait_changed(GtkEntry *e){
   *recesiveT = '\0';
   strcat(recesiveT,gtk_entry_get_text(e));


}

void on_btn_saveTrait_clicked(){

    if(checkError == 1){
        gtk_list_store_append (store, &iter);

      
        gtk_list_store_set (store, &iter,
                    COL_GEN, dominantL,
                    COL_DOMINANT, dominantT,
                    COL_RECESIVE,recesiveT,
                    -1);
        insertArray(alpha, char, dominantL[0]);
        genesFinal += 1;
        on_btn_saveFile(dominantL,dominantT,recesiveT);
        gtk_widget_show_all (windowTable);

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
    strList = generateGenotypes(alpha);

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
  grid_mendel = GTK_WIDGET(gtk_builder_get_object(builder, "grid_mendel"));

  int maxLet = strlen(fatherTxt)/2;
  int total = (int)pow(2.0, (double)maxLet);
  if(grid_mendelRows != total){
    if(grid_mendelRows != 0 && grid_mendelRows > total){
      gtk_widget_destroy(grid_mendel);
      grid_mendel = gtk_grid_new();
      GtkWidget *vport_mendel = GTK_WIDGET(gtk_builder_get_object(builder, "vport_mendel"));
      gtk_container_add(GTK_CONTAINER(vport_mendel), grid_mendel);
      gtk_widget_show(GTK_WIDGET(grid_mendel));
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
  for(int i = 0; i <= total; i++){
    GtkWidget* cell = gtk_grid_get_child_at(GTK_GRID(grid_mendel), i, i);
    if(!cell){
      gtk_grid_insert_row(GTK_GRID(grid_mendel), i);
      gtk_grid_insert_column(GTK_GRID(grid_mendel), i);
    }
  }
  for(int row = 0; row <= total; row++){
    for(int col = 0; col <= total; col++){
      GtkWidget* cell = gtk_grid_get_child_at(GTK_GRID(grid_mendel), col, row);
      if(!cell){
        cell = gtk_frame_new("");
        gtk_frame_set_label_align(GTK_FRAME(cell), 0.5, 0.5);
        gtk_grid_attach(GTK_GRID(grid_mendel), cell, col, row, 1, 1);
        gtk_widget_show(cell);
      }
    }
  }
  
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


