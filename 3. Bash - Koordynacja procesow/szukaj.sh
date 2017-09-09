#!/bin/bash

if [[ $# -lt 2 ]] || [[ $# -gt 3 ]]; then
   echo "Błędna liczba argumentów ($#), spodziewane 2 argumenty: plik katalog"
   exit 0
elif [[ $# -eq 3 ]] && [[ $3 != true ]]; then
   echo "Spodziewane 2 argumenty: plik katalog"
   exit 0
fi

declare -i foo

file_to_find=$1

if [[ -d $2 ]]; then
   catalog=$2
else
   echo "Nie znaleziono katalogu"
   exit 0
fi

files=`ls -1 $catalog`
match_count=0

for file in $files; do
   if [[ $file == $file_to_find ]]
   then
      echo "$catalog/$file"
      let "match_count++"
   elif [[ -d $catalog/$file ]]
   then
      $0 $file_to_find $catalog/$file true &
   fi
done

# -p List only the process of the job’s process group leader
for job in `jobs -p`; do
   wait $job
   let "match_count+=$?"
done

# not first recursion, 3rd arg set
if [[ -n $3 ]]; then
   exit $match_count
elif [[ $match_count -eq 0 ]]; then
   echo "Nie znaleziono plików dla podanej nazwy: $file_to_find."
fi

