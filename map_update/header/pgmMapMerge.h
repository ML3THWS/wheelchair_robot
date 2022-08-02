#ifndef pgmMapMerge_H
#define pgmMapMerge_H

#include "pgmMapCheck.h"
#include <array>

using std::string;
using std::vector;
using std::array;

/*class for comparing different pgm map files generated from the pgmMapCompare class
on different days and using these generated maps in order to merge the same "new"
or removed objects into one new map of the environment
REMINDER: THE MAP WHICH WILL BE GENERATED BY THIS CLASS ONLY ADDS NEW OBJECTS
OR REMOVES OLD OBJECTS, IF A NEW OBJECT HAS BEEN DETECTED AND ADDED BY THE pgmMapCompare
CLASS MULTIPLE DAYS IN A ROW OR IF A MISSING OBJECTS HASE BEEN MISSING IN THE pgmMapCompare
CLASS MULTIPLE DAYS IN A ROW
*/
//INFO: In pgm files stored with map_server node the grid values are 254=free, 205=unknown and 0=occupied

class pgmMapMerge
{
	private:
	//general parameters of the class
	vector<vector<bool>> changable_grid_matrix;			//pointer to the matrix showing which grids are changable
	vector<vector<uint8_t>> initial_map;				//loaded base/initial map
	vector<vector<uint8_t>> resulting_map;				//variable to store the resulting map
	uint8_t count_maps_to_compare;					/*variable to store the count of days which should be used 
									therefore the count of maps to use, to merge into one final map*/
	vector<vector<vector<uint8_t>>> maps_to_merge;			/*vector to store all the maps generated by the pgmMapCompare class 
									which should be used to search for missing or new objects for multiple
									days in a row in order to generate the final map by this pgmMapMerge class*/
	
	string path_to_initial_map_yaml_file;				//safe path to initial yaml file from the base/initial map
	
	//define private functions
	bool check_compatible_map_sizes();				//check if all loaded maps have the same x and y size (otherwise there is an error)
	void merge_compared_maps();					//merge all the given maps which have been generated by the pgmMapCompare class		
	void find_new_object_grid_cell_groups(array<uint16_t, 2> coord);//find group of grid cells which belong to one object and mark/input them into the resulting map
	void find_removed_objects_grid_cell_groups(array<uint16_t, 2> coord); 	//find group of grid cells which belong to one object and mark/input them into the resulting map
	uint8_t check_cell_in_maps_to_merge(uint16_t y, uint16_t x); 	/*function which checks if all cells at a given coordinate have the same value.
									if this is not the case the function gives back the cell value which differentiates from
									the cell value at the given coordinate from maps_to_merge.at(0)*/
	uint8_t check_for_unoccupied_cell_in_maps_to_merge(uint16_t y, uint16_t x); 	/*We use this function in order to search if ONE and only one of the maps
											used in maps_to_merge has an free/unoccupied cell at the given coordinate*/
	bool check_count();

	public:
	//define constructor and destructor
	pgmMapMerge(pgmMapCheck *passed_initial_map, vector<pgmMapCheck> *passed_maps, string initial_map_yaml_file_path);
	~pgmMapMerge();

	//public functions
	vector<vector<uint8_t>>  merge_result_map();
	vector<vector<uint8_t>>  merge_result_with_edit(pgmMapCheck *initial_map_with_edit);	//function for getting the resulting map but with edited objects from (e.g. glass or tables) to overwrite the initial map
	uint8_t get_days_to_compare();
	bool write_new_updated_map_pgm_and_yaml(string path_to_new_map_file_loc, string name_of_new_map);
};

#endif