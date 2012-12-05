Parangon
========

par - The parangon archiving utility

  	
SYNOPSIS

		par <operation> [options]

		Operations:
		-h
		-t
		-r
		-c <files | directories>
		-u
		-x
		-z
		-d <file>
		-sparse
		-m

		Options:
		-f <output file>
		-v
		-z

		
DESCRIPTION

		A simple file archiving tool.

		
EXAMPLES

		par -xvf foo.par
		verbosely extract foo.par

		par -xzf foo.par.gz
		extract gzipped foo.par.gz

		par -xzf foo.par.gz blah.txt
		extract the file blah.txt from foo.par.gz


FUNCTION LETTERS

		One of the following options must be used:
		
		-h
		display the help
		
		-t
		list the contents from an archive
		
		-r <files | directories>
		append files (or directories) to the end of an archive
		
		-c <files | directories>
		create an archive from files (and directories)
		
		-u
		only append files that are newer than the existing in archive
		
		-x
		extract files from an archive
		
		-d <file>
		delete file from the archive (not for use on magnetic tapes !)
		
		-sparse
		
		-m
		find differences between archive and file system

		
OPTIONS

		-f <file>
		use archive file or device F (default "-", meaning stdin/stdout)

		-v
		verbosely list files processed

		-z
		filter the archive through gzip