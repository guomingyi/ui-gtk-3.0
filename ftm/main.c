#include <stdio.h>
#include <gtk/gtk.h>

#include "usb.h"

int main(int argc,char *argv[])
{
    gtk_init(&argc,&argv);
    GtkBuilder *builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder,"builder.ui", NULL)) {
        printf("connot load builder.ui file!");
		return -1;
    }

    gtk_main();
    return 0;
}
