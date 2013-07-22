#!/usr/bin/perl
use warnings;
use strict;

my $filename = $ARGV[0];
open(my $fh, $filename) or die $!;
my @lines = <$fh>;
close($fh);
my $numlines = scalar @lines;

print <<EOT
#include <syslog.h>
#include <stdio.h>
#include "pcre.h"
#include "request.h"
#include "route.h"
#include "http_parser.h"

int GNUM_ROUTES;

EOT
;

foreach(@lines){
    my @list = split(/\s+/);
    print "int $list[2](int, struct message*, char **splat, int splat_len);\n";
}
print "\n";

print <<EOT
int setup_routes()
{
    pcre            *re;
    char            *pattern;
    const char      *errmsg;
    int             erroffset;
    route_t         *myroutes;
    int             i;

    GNUM_ROUTES = $numlines;

    myroutes = calloc($numlines, sizeof(route_t));

EOT
;


my $count = 0;
foreach(@lines){
    chomp();
    my @list = split(/\s+/); 
    print "    /*********************************************************/\n";
    print "    /* $_ */\n";
    print "    /*********************************************************/\n";
    print "    myroutes[$count].method = HTTP_" . uc($list[0]) . ";\n"; 
    print "    pattern = \"$list[1]\";\n";
    print "    re = pcre_compile( pattern,\n";
    print "                       0,\n";
    print "                       &errmsg,\n";
    print "                       &erroffset,\n";
    print "                       NULL);\n";
    print "\n";
    print "    if(re == NULL)\n";
    print "    {\n";
    print "        syslog(LOG_CRIT, \"%s(): PCRE compilation failed at offset %d with error '%s' in pattern '%s'\", __func__, erroffset, errmsg, \"$list[1]\");\n";
    print "        goto cleanup;\n";
    print "    }\n";
    print "    myroutes[$count].re = re;\n";
    print "    myroutes[$count].handler = $list[2];\n";
    print "\n";

    $count++;
}

print <<EOT
    goto end;

cleanup:
    for(i = 0; i < $count; i++)
    {
        free(myroutes[i].re);
    }
    free(myroutes);
    GROUTES = NULL;
    return 1;

end:
    GROUTES = myroutes;
    return 0;
}

EOT
;   
