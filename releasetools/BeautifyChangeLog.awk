# Add title to page
# Add bars marking versions to ChangeLog.html
# Include stylesheet NestChangeLog.css directly in file

# replace stylesheet link with file
/<link rel="stylesheet" href="svn2html.css" type="text\/css"\/>/ {
  print "<style type=\"text/css\">"
  system("cat " cssfile)
  print "</style>"

  # drop original line
  next
}

# add heading
/<body>/ { print $0
           print "<h1>NEST Revision History</h1>" 
           print "<h2>NEST Version " nestversion "</h2>"
	   #drop original line
           next
}

# insert headings for each version---up to and including 1.9.11
/<span class="changelog_message">Hiked patchlevel to nest-[0-9]+\.[0-9]+\.[0-9]+<\/span>/ {
  npos = match($0, "<span class=\"changelog_message\">Hiked patchlevel to nest-[0-9]+.[0-9]+.[0-9]+</span>");
  tmp = substr($0, npos);
  nmpos = match(tmp, "nest-");
  sppos = match(tmp, "</span>");
  vnr = substr(tmp, nmpos+5, sppos-nmpos-5);
  print "</ul>"
  print ""
  print "<H2>NEST Version " vnr "</H2>"
  print ""
  print "<ul class=\"changelog_entries\">"
}

# copy all lines
{ print }
