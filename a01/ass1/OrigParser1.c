ICalErrorCode createCalendar(char* fileName, Calendar** obj) {
	*obj = NULL;

	// check file name
	if(fileName == NULL) {
		ICalErrorCode toReturn = INV_FILE;
		return toReturn;

	} else {
		int length = strlen(fileName);
		if(fileName[length-4] != '.' || fileName[length-3] != 'i' || fileName[length-2] != 'c' || fileName[length-1] != 's') {
			ICalErrorCode toReturn = INV_FILE;
			return toReturn;
		}
	}

	FILE *fp;
	int len = 0;
	char *entireFile;

	// read file all at once if it exists
	fp = fopen(fileName, "r");

	if(fp == NULL) {
		ICalErrorCode toReturn = INV_FILE;
		return toReturn;

	} else {
		fseek(fp, 0, SEEK_END); // moves to end of file
		len = ftell(fp); // calculates number of chars (using total bytes)
		fseek(fp, 0, SEEK_SET); // moves it back to beginning

		entireFile = calloc(len+1, sizeof(char)); // init the array
		fread(entireFile, sizeof(char), len, fp); // reads entire file into the array at once
		entireFile[len] = '\0';
	}
	fclose(fp);

	// ---------------------------------- parse char by char ------------------------------------
	
	/*int numCR = 0;
	int numLF = 0;
	int n = 0;
	int numHTAB = 0;

	for(int i = 0; i < strlen(entireFile); i++) {
		if(entireFile[i] == CR) {
			//printf("'CR'");
			numCR++;
		} else if(entireFile[i] == LF) {
			//printf("'LF'");
			numLF++;
		} else if(entireFile[i] == HTAB) {
			//printf("'HTAB'");
			numHTAB++;
		} else if(entireFile[i] == '\n') {
			//printf("'\n'");
			n++;
		} else {
			//printf("%c", entireFile[i]);
		}
	}

	// testing some character counts
	
	printf("\n\n\tnumCR = %d\n\tnumLF = %d\n\t#\\n = %d\n\tnumHTAB = %d\n", numCR, numLF, n, numHTAB);*/

	printf("\n\tBEFORE: entireFile:\n%s\n", entireFile);

	// ------------------ UNFOLD --------------------

	int numFoldedLines = 0;
	char *temp = calloc(strlen(entireFile)+1, sizeof(char));
	char *ending = calloc(strlen(entireFile)+1, sizeof(char));

	// search for char sequence indicator of folded line
	for(int i = 0; i < strlen(entireFile); i++) {
		if( i < strlen(entireFile)-2 && entireFile[i] == CR && entireFile[i+1] == LF && (entireFile[i+2] == SPACE || entireFile[i+2] == HTAB) ) {
			entireFile[i] = entireFile[i+1] = entireFile[i+2] = '^';
			numFoldedLines++;

			// get first half
			//temp = realloc(temp, strlen(entireFile)-2);
			strncpy(temp, entireFile, i);
			temp[strlen(temp)] = '\0';
			// get second half
			//ending = realloc(ending, strlen(entireFile)-i);
			strcpy(ending, entireFile+i+3);
			ending[strlen(ending)] = '\0';
			// put back
			strcat(temp, ending);
			strcpy(entireFile, temp);

			/*printf("\n\ttemp @index=%d:\n%s\n", i, temp);
			printf("\n\tending @index=%d:\n%s\n", i, ending);*/

		} else if( i < strlen(entireFile)-1 && entireFile[i] == CR && entireFile[i+1] != LF) {
			// free everything, retun INV_FILE error

			printf("\t--CR but no LF\n"); // TODO

		} else if( i > 0 && entireFile[i] == LF && entireFile[i-1] != CR) {
			// free everything, retun INV_FILE error

			printf("\t--LF but no CR\n"); // TODO

		}
	}
	free(temp);
	free(ending);

	printf("\n\tnumFoldedLines = %d\n", numFoldedLines);

	printf("\n\tAFTER: entireFile:\n%s\n", entireFile);

	// ---------------------- READ -----------------------

	// create mem since it's a good file so far
	//*obj = realloc(*obj, sizeof(Calendar));

	/*char *line = calloc(strlen(entireFile)+1, sizeof(char));
	line[strlen(line)] = '\0';

	char *contentLine = calloc(strlen(entireFile)+1, sizeof(char));
	contentLine[strlen(contentLine)] = '\0';

	bool inCal = false;

	bool newLine = true;
	// loop through file, read things in					----------------------- strtok by CRLF instead ?
	for(int i = 0; i < strlen(entireFile); i++) {
		// check for end of content line
		if( i+1 < strlen(entireFile) && entireFile[i] == CR && entireFile[i+1] == LF ) {
			// full content line is loaded, put it in the new string
			
			free(contentLine);
			contentLine = calloc(strlen(line)+1, sizeof(char));
			strcpy(contentLine, line);
			contentLine[strlen(contentLine)] = '\0';
			
			// check if comment
			if(line[0] == ';') {
				// do nothing, it's a comment
			} else {
				// check for BEGIN:VCALENDAR
				if( strcmpic(strtok(contentLine, ":"), "BEGIN") == 0 && strcmpic(strtok(NULL, ":"), "VCALENDAR") == 0 ) {
					inCal = true;

					// **********************************************************************************************************************
					printf("\tBEGIN:VCALENDAR was found: '%s'\n", line);
					printf("\tBEGIN:VCALENDAR was found: '%s'\n", contentLine);

				} else if(inCal) {
					printf("inCal line = '%s'\n", line);
					printf("inCal line = '%s'\n", contentLine);
					// get version and product id
					if( strcmpic(strtok(contentLine, ":"), "VERSION") == 0 ) {
						// store next token as the version number (float)
						printf("line = '%s'\n", line);
						printf("line = '%s'\n", contentLine);
						// **********************************************************************************************************************

					} else if( strcmpic(strtok(contentLine, ":"), "PRODID") == 0 ) {
						// store the next token as the product id (string)
						
						// **********************************************************************************************************************

					}

				} else {
					printf("\tWHOOPS!\n");
				}
			}

			printf("\t@i=%d--%s\n", i, line);
			newLine = true;
			i++;
		} else {
			if(newLine == true) {
				newLine = false;
				// restart the line string
				free(line);
				line = calloc(strlen(entireFile)+1, sizeof(char));
				strncpy(line, entireFile+i, 1);
				line[strlen(line)] = '\0';

			} else {
				// append to the line string
				strncat(line, entireFile+i, 1);
			}

			//printf("@i=%d--%s\n", i, line);

		}
	}

	free(line);
	free(contentLine);*/

	// create mem since it's a good file so far
	*obj = realloc(*obj, sizeof(Calendar));
	
	/*// create folder for content lines
	char *contentLine = calloc(strlen(entireFile)+1, sizeof(char));
	contentLine[strlen(contentLine)] = '\0';*/

	// create holder for strtok
	char *token = strtok(entireFile, "\r\n");

	int i = 0; // can delete later, jsut line iterator for while loop

	// init booleans
	bool inCal = false;
	bool version = false;
	bool prodID = false;

	bool inEvent = false;
	bool inAlarm = false;

	// continue until all content lines are read
	while(token != NULL) {

		printf("A-Line %d: '%s'\n", i, token);

		/*// ready the content line to manipulate
		free(contentLine);
		contentLine = calloc(strlen(token)+1, sizeof(char));
		//strcpy(contentLine, token);
		contentLine[strlen(contentLine)] = '\0';*/

		// loop through content line char by char
		/*for(int j = 0; j < strlen(token); j++) {
			contentLine[j] = token[j];
			printf("--contentLine @j=%d--'%s'\n", j, contentLine);
		}*/

		// must look for start of cal first, ignore comment lines
		if(token[0] == ';') {
			// ignore comment, do nothing

		} else if(!inCal) { // add INV_CAL error return where loop goes through whole file without finding either below -TODO
			if(strcmpic(token, "BEGIN:VCALENDAR") == 0) {
				inCal = true;

				printf("we are now inCal\n");

			} else if(strcmpic(token, "END:VCALENDAR") == 0) {
				// free everything, return INV_CAL error TODO

				printf("not supposed to happen: end cal reached before begin cal\n");
			}

		} else { // inCal
			// check BEGIN:
			if(strncmpic(token, "BEGIN:", 6) == 0) {
				if(strcmpic(token+6, "VCALENDAR") == 0) {
					// free everything, return DUP_CAL error TODO

					printf("not supposed to happen: begin cal found w/in cal\n");

				} else if(strcmpic(token+6, "VEVENT") == 0) {
					inEvent = true;

					printf("we are now inEvent\n");

				} else if(strcmpic(token+6, "VALARM") == 0) {
					inAlarm = true;

					printf("we are now inAlarm\n");

				} else {
					// free everything, return error TODO

					printf("not supposed to happen: found a different BEGIN:etc '%s'\n", token+6);

				}
			}
			// check END:
			else if(strncmpic(token, "END:", 4) == 0) {

			}
			// check calendar properties
			else if(strncmpic(token, "VERSION:", 8) == 0) {
				version = true;

				char *ptr;
				(*obj)->version = (float)strtod(token+8, &ptr);

				printf("found version = '%s' = %f ; w/ leftovers: '%s'\n", token+8, (*obj)->version, ptr);

				if(strcmp(ptr, "") != 0) {
					// free everything, return INV_VER error
					free(entireFile);
					ICalErrorCode toReturn = INV_VER;
					return toReturn;
				}

			} else if(strncmpic(token, "PRODID:", 7) == 0) {
				prodID = true;
				strcpy((*obj)->prodID, token+7);

				printf("found prodid = '%s'\n", (*obj)->prodID);

			} else {
				// it's a property of the iCal??
			}
		}

		printf("B-Line %d: '%s'\n", i, token);

		// iterate
		token = strtok(NULL, "\r\n");
		i++;
	}

	//free(contentLine);
	
	free(entireFile);
	ICalErrorCode toReturn = OK;
	return toReturn;
}