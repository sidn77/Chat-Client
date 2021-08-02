#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>


typedef struct
{
     GtkWidget *entry, *textview;
} Widgets;

char u_id[50];
int flag = 0;
GtkTextMark *mark;

static void insert_text (GtkButton*, Widgets*);
static void s_file();
void client(const gchar* text);
void* client_1(void*);
static void insert_text_1 (const gchar*, Widgets*);
static void destroy(GtkWidget*, gpointer);
static void send_file ();


int main ()
{	
	 pthread_t thread1, thread2;
	 int iret1, iret2;
     GtkWidget *window, *scrolled_win, *hbox, *vbox, *insert, *scrolled_win_1, *quit, *sendf;
     Widgets *w = g_slice_new (Widgets); 

     gtk_init (NULL, NULL);

     window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
     gtk_window_set_title (GTK_WINDOW (window), "ChatBox");
     gtk_container_set_border_width (GTK_CONTAINER (window), 10);
     gtk_widget_set_size_request (window, -1, 200);

     w->textview = gtk_text_view_new ();
     w->entry = gtk_entry_new ();
     insert = gtk_button_new_with_label ("Send");
     sendf = gtk_button_new_with_label ("Send File");
     quit = gtk_button_new_with_label ("Quit");

     g_signal_connect (G_OBJECT (insert), "clicked", G_CALLBACK (insert_text), (gpointer) w);
     g_signal_connect(G_OBJECT(sendf), "clicked", G_CALLBACK(send_file), (gpointer) w);
     g_signal_connect(G_OBJECT(quit), "clicked", G_CALLBACK(destroy), (gpointer) w);

     scrolled_win = gtk_scrolled_window_new (NULL, NULL);
     gtk_container_add (GTK_CONTAINER (scrolled_win), w->textview);
     gtk_widget_set_hexpand (scrolled_win, TRUE);
     gtk_widget_set_vexpand (scrolled_win, TRUE);
     gtk_text_view_set_editable(GTK_TEXT_VIEW(w->textview), FALSE);

     hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
     gtk_box_pack_start (GTK_BOX (hbox), w->entry, TRUE, TRUE, 0);
     gtk_box_pack_start (GTK_BOX (hbox), insert, TRUE, TRUE, 0);
     gtk_box_pack_start (GTK_BOX (hbox), sendf, TRUE, TRUE, 0);
     gtk_box_pack_start (GTK_BOX (hbox), quit, TRUE, TRUE, 0);

     

     vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
     gtk_box_pack_start (GTK_BOX (vbox), scrolled_win, TRUE, TRUE, 0);
     gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

     gtk_container_add (GTK_CONTAINER (window), vbox);
     gtk_widget_show_all (window);

 	 bzero(u_id, 50);
     printf("\n-Login-\nusername: ");
	 scanf("%s", u_id);
 	 pthread_create(&thread2, NULL, client_1, (void *)w);
 	 gtk_main();     
     
     return 0;
}


static void send_file() {
	/*
		Spawns UI to send the file.
	*/

	GtkWidget *window, *grid, *label, *entry, *send;
	Widgets *w = g_slice_new (Widgets);
  	gtk_init(NULL, NULL);

  	/* Spawns a new empty window */

  	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "File Transfer");
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    gtk_widget_set_size_request (window, -1, 200);	

    /* Spawns grid Layout */

   	grid = gtk_grid_new ();
   	gtk_container_add (GTK_CONTAINER (window), grid);

  	label = gtk_label_new ("Enter File Name:" );
  	gtk_grid_attach (GTK_GRID (grid), label, 1, 1, 1, 1);

	w->entry = gtk_entry_new ();
  	gtk_grid_attach (GTK_GRID (grid), w->entry, 3, 1, 1, 1);

  	send = gtk_button_new_with_label ("Send");
  	gtk_grid_attach (GTK_GRID (grid), send, 1, 2, 1, 1);
  	g_signal_connect(G_OBJECT(send), "clicked", G_CALLBACK(s_file), (gpointer) w);


    gtk_widget_show_all(window);

  	gtk_main ();
}


static void s_file (GtkButton *button, Widgets *w) {

	/* 

		Send the file using FTP

	*/

	const gchar *text = gtk_entry_get_text (GTK_ENTRY (w->entry));
	char *buff = (char*) text;
	char con[256]; 
	struct sockaddr_in serv;
	int n, s;
	s = socket(AF_INET, SOCK_STREAM, 0);

	/* Initializes server socket */
	serv.sin_family = AF_INET;
        serv.sin_port = htons(8888);
    	serv.sin_addr.s_addr = inet_addr("192.168.43.250");

	n = connect(s, (struct sockaddr*)&serv, (sizeof(serv)));
	int f = open(buff, 0);
	if(f < 0)
	{
		strcpy(con, "Error. File doesn't exist.");
		puts(con);
		n = send(s, (char*)con, 255, 0);			
	}
	else
	{
		strcpy(con, "Incoming File."); 
		n = send(s, (char*)con, 255, 0);
		n = send(s, (char*)u_id, 50, 0);
		n = send(s, (char*)buff, 255, 0);	
		do
		{
			bzero(con, 256);				
			n = read(f, (char*)con, 255);
			int r = send(s, (char*)con, n, 0);
		}while(n > 0);
		printf("\nSent.");
		strcpy(con, "~");
		n = send(s, (char*)con, 255, 0);
	}
}


static void insert_text (GtkButton *button, Widgets *w)
{
	/*
		Inserts text from entry space to textview.
	*/

     GtkTextBuffer *buffer;
     GtkTextMark *mark;
     GtkTextIter iter;
     const gchar *text;

     buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w->textview));
     text = gtk_entry_get_text (GTK_ENTRY (w->entry));
 	 client(text);
}


void client(const gchar* text)
{	
	/* 
		Sends the Message using TCP. 
	*/

	struct sockaddr_in serv;
	int s, n, ch;
	char *buff, ebuff[256], *con;
	buff = (char*) malloc (256 * sizeof(char));
	con = (char*) malloc (256 * sizeof(char));

	bzero(buff, 256);
	bzero(ebuff, 256);
	s = socket(AF_INET, SOCK_STREAM, 0);
	serv.sin_family = AF_INET;
    	serv.sin_port = htons(8888);
		serv.sin_addr.s_addr = inet_addr("192.168.43.250");
	n = connect(s, (struct sockaddr*)&serv, (sizeof(serv)));
	buff = (char *)text;
	strcpy(con, u_id);
	strcat(con, ": ");
	strcat(con, buff);
	n = send(s, (char*)con, 255, 0);
	flag = 1;
	
}


static void insert_text_1 (const gchar* text, Widgets *w)
{	 
	 /*
		Takes the text from the recieved buffer Inserts text into the textview.
	 */

     GtkTextBuffer *buffer;
     
     GtkTextIter iter;

     buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w->textview));

     mark = gtk_text_buffer_get_insert (buffer);
     gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
     if (gtk_text_buffer_get_char_count(buffer))
    	gtk_text_buffer_insert (buffer, &iter, "\n", 1);
     gtk_text_buffer_insert (buffer, &iter, text, -1);
}


void* client_1(void* params)
{	
	/*
		Recieves both file and message from server.	
	*/

	struct sockaddr_in serv;
	Widgets *w = (Widgets *)params;
	int s, n, ch;
	char *ebuff;
	ebuff = (char *) malloc(256 * sizeof(char));
	bzero(ebuff, 256);
	char buff[50], c;
		s = socket(AF_INET, SOCK_STREAM, 0);
		serv.sin_family = AF_INET;
        	serv.sin_port = htons(8888);
    		serv.sin_addr.s_addr = inet_addr("192.168.43.250");
		n = connect(s, (struct sockaddr *)&serv, (sizeof(serv)));
		while(1) {
			n = recv(s, (char*)ebuff, 255, 0);
			if(strcmp("Incoming File.", ebuff) == 0) {
				bzero(ebuff, 256);
				bzero(buff, 50);
				int r, c, n, f, e;
				printf("\nDetails-");
				printf("\nUser ID: ");
				r = recv(s, (char*)buff, 50, 0);
				printf("%s", buff);
				printf("\nFile Name: ");
				r = recv(s, (char*)ebuff, 255, 0);
				printf("%s", ebuff);
				printf("\nPress (1)to save, (2)to view and (0)to reject.");
				scanf("%d", &c);
				if(!c)
				{
					printf("\nRejected.");
				}		
				else {
					if(c == 1) {
						bzero(ebuff, 256);
						printf("\nSave as?");
						scanf("%s", ebuff);
						f = open(ebuff, O_RDWR|O_CREAT,S_IRWXU);
					}
					e = 1;
					do {
						bzero(ebuff, 256);
						r = recv(s, (char*)ebuff, 255, 0);
						if(c == 1)
							n = write(f, ebuff, r);
						else
							puts(ebuff);
						if(ebuff[strlen(ebuff) - 1] == '~')
							e = 0;
					}while(e);
					if(c)
						close(f);
				}
			}
			else {
				insert_text_1((const gchar*)ebuff, w);
			}
			
			printf("%s", ebuff);
		}
}


static void destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();
}
