#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NUM 10

typedef struct name {
	char* firstName;
	char* lastName;
	unsigned int age;
} Person;

//*********** Comparator functions **************

//All functions must compare two elements of the same type

// compareFirstName returns true if two names have the same first name
bool compareFirstName(const void *first, const void *second){
    Person* tmpName1;
    Person* tmpName2;
    
    if (first == NULL || second == NULL){
        return false;
    }
    
    tmpName1 = (Person*)first;
    tmpName2 = (Person*)second;
    
    //Elements are "equal" if their first names are equal
    if (strcmp(tmpName1->firstName, tmpName2->firstName) == 0){
        return true;
    }else{
        return false;
    }
}

// compareFirstName returns true if two names have the same last name
bool compareLastName(const void *first, const void *second){
    Person* tmpName1;
    Person* tmpName2;
    
    if (first == NULL || second == NULL){
        return false;
    }
    
    tmpName1 = (Person*)first;
    tmpName2 = (Person*)second;
    
    //Elements are "equal" if their last names are equal
    if (strcmp(tmpName1->lastName, tmpName2->lastName) == 0){
        return true;
    }else{
        return false;
    }
}

// compareFirstName returns true if two names have the same age
bool compareAge(const void *first, const void *second){
    Person* tmpName1;
    Person* tmpName2;
    
    if (first == NULL || second == NULL){
        return false;
    }
    
    tmpName1 = (Person*)first;
    tmpName2 = (Person*)second;
    
    //Elements are "equal" if their ages are equal
    return tmpName1->age == tmpName2->age;
}
//*******************************************************

// Search function
Person* search(Person* array[],
               int arrayLen,
               bool (*compare)(const void*, const void*),
               Person* searchName){
    
    if (compare == NULL || searchName == NULL){
        return NULL;
    }
    
    for (int i = 0; i < arrayLen; i++){
        //Compare an array element to the "search name" - i.e. Name struct with the search criteria
        //using the provided comparator fuction
        if (compare((const void*)array[i], (const void*)searchName)){
            //found a match
            return array[i];
        }
    }
    //If we got here, none of the array elements matched the search criteria
    return NULL;
}

int main(void){
    Person* nameArray[NUM];
    char tmpStr[100];
    
    
    //Fill array with values
    for (int i = 0; i < NUM; i++){
        int memLen;
        nameArray[i] = malloc(sizeof(Person));
        
        nameArray[i]->age = (i+1)*10;
        
        sprintf(tmpStr, "Name%d", i);
        memLen = strlen(tmpStr)+2;
        nameArray[i]->firstName = (char*)malloc(sizeof(char)*memLen);
        strcpy(nameArray[i]->firstName, tmpStr);
        
        sprintf(tmpStr, "Lastname%d", i);
        memLen = strlen(tmpStr)+2;
        nameArray[i]->lastName = (char*)malloc(sizeof(char)*memLen);
        strcpy(nameArray[i]->lastName, tmpStr);
    }
    
    Person searchName;
    searchName.firstName = malloc(200*sizeof(char));
    searchName.lastName = malloc(200*sizeof(char));
    Person* result = NULL;
    
    
    
    //Search by first name - name exists
    
    strcpy(searchName.firstName, "Name2");
    
    result = search(nameArray, NUM, compareFirstName, &searchName);
    if (result != NULL){
        printf("Found element %s %s %d\n", result->firstName, result->lastName, result->age);
    }else{
        printf("Element with %s not found\n", searchName.firstName);
    }
    
    
    
    //Search by first name - name does not exist
    
    strcpy(searchName.firstName, "Rick");
    result = search(nameArray, NUM, compareFirstName, &searchName);
    if (result != NULL){
        printf("Found element %s %s %d\n", result->firstName, result->lastName, result->age);
    }else{
        printf("Element with %s not found\n", searchName.firstName);
    }
    
    
    //Search by last name - name exists
    
    strcpy(searchName.lastName, "Lastname7");
    result = search(nameArray, NUM, compareLastName, &searchName);
    if (result != NULL){
        printf("Found element %s %s %d\n", result->firstName, result->lastName, result->age);
    }else{
        printf("Element with %s not found\n", searchName.lastName);
    }
    
    //Search by last name - name does not exist
    
    strcpy(searchName.lastName, "Sanchez");
    result = search(nameArray, NUM, compareLastName, &searchName);
    if (result != NULL){
        printf("Found element %s %s %d\n", result->firstName, result->lastName, result->age);
    }else{
        printf("Element with %s not found\n", searchName.lastName);
    }
    
    //Bad searching - NULL pointers
    
    printf("Let's try searching with NULL pointers...\n");
    strcpy(searchName.lastName, "Sanchez");
    result = search(nameArray, NUM, NULL, &searchName);
    if (result != NULL){
        printf("Found element %s %s %d\n", result->firstName, result->lastName, result->age);
    }else{
        printf("Element with %s not found\n", searchName.lastName);
    }
    
    strcpy(searchName.lastName, "Sanchez");
    result = search(nameArray, NUM, compareLastName, NULL);
    if (result != NULL){
        printf("Found element %s %s %d\n", result->firstName, result->lastName, result->age);
    }else{
        printf("Element with %s not found\n", searchName.lastName);
    }
    
    //********************* Free all allocated data *********************
    free(searchName.firstName);
    free(searchName.lastName);
    
    for (int i = 0; i < NUM; i++){
        free(nameArray[i]->firstName);
        free(nameArray[i]->lastName);
        free(nameArray[i]);
    }
    return 0;
}
