# this will iterate through all directories in your current folder and run
# cartogram_cpp code with original.json and template.csv
# assuming that ./cartogram is aliased to cartogram_cpp

# copy paste in to your terminal
# replace original.json and template.csv with your data

# to test data
for d in */ ; do
  printf "Processing ${d}...\n"
  printf "=== ${d} ===\n\n" >> test_data.txt
  cartogram_cpp -g ${d}original.json -v ${d}template.csv >> test_data.txt
  printf "\n\n" >> test_data.txt
  printf "Done processing ${d}\n\n"
done

# to change data back to backup
for d in */ ; do
  mv ${d}template.csv.bak ${d}template.csv
done

# to change csv to NAME_1
for i in */template.csv; do sed -i.bak '1 s/[A-Za-z0-9]*,/NAME_1,/' $i; done
