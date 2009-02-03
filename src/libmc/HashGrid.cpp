#include "HashGrid.h"

HashGrid::HashGrid(string filename, float v, float scale){

  //Initalize global variables
  voxelsize = v;

  xmin = 1.0e38;
  ymin = 1.0e38;
  zmin = 1.0e38;

  xmax = -xmin;
  ymax = -ymin;
  zmax = -zmin;

  //Read Data
  cout << "##### Reading data..." << endl;
  number_of_points = readPoints(filename, scale);

  //Create Distance Function
  //distance_function = new DistanceFunction(points, number_of_points, 10, false);
  interpolator = new PlaneInterpolator(points, number_of_points, 10.0, 100, 100.0);

  //Create Grid
  createGrid();
  //createGridMT();
  //Create Mesh
  createMesh();
  //createMeshMT();

  cout << "SAVE" << endl;
  mesh.save("mesh.ply");
  cout << "-----" << endl;
}

HashGrid::~HashGrid(){

  annDeallocPts(points);
  cells.clear();
}

void HashGrid::createGrid(){

	//Create Grid
	cout << "##### Creating Grid..." << endl;

	//Current indices
	int index_x, index_y, index_z;
	int dx, dy, dz;
	int hash_value;


	//Iterators
	hash_map<int, Box*>::iterator it;
	hash_map<int, Box*>::iterator neighbour_it;

	for(int i = 0; i < number_of_points; i++){


		index_x = calcIndex((points[i][0] - xmin) / voxelsize);
		index_y = calcIndex((points[i][1] - ymin) / voxelsize);
		index_z = calcIndex((points[i][2] - zmin) / voxelsize);


		for(int j = 0; j < 8; j++){

			dx = HGCreateTable[j][0];
			dy = HGCreateTable[j][1];
			dz = HGCreateTable[j][2];

			//     for(dx = -1; dx < 1; dx++)
			// 	 for(dy = -1; dy < 1; dy++)
			// 	   for(dz = -1; dz < 1; dz++){

			//dx = dy = dz = 0;

			hash_value = hashValue(index_x + dx,
					index_y + dy,
					index_z + dz);

			it = cells.find(hash_value);
			if(it == cells.end()){
				Box* new_box = new Box(Vertex((index_x + dx) * voxelsize + xmin,
						(index_y + dy) * voxelsize + ymin,
						(index_z + dz) * voxelsize + zmin), voxelsize);
				cells[hash_value] = new_box;
			}
		}


	}

  cout << "##### Finished Grid Creation. Number of generated cells: " << cells.size() << endl;
}

void HashGrid::createGridMT(){

	//Create Grid
	cout << "##### Creating MT-Grid..." << endl;

	//Current indices
	int index_x, index_y, index_z;
	int dx, dy, dz;
	int hash_value;


	//Iterators
	hash_map<int, TetraederBox*>::iterator it;
	hash_map<int, TetraederBox*>::iterator neighbour_it;

	for(int i = 0; i < number_of_points; i++){


		index_x = calcIndex((points[i][0] - xmin) / voxelsize);
		index_y = calcIndex((points[i][1] - ymin) / voxelsize);
		index_z = calcIndex((points[i][2] - zmin) / voxelsize);


		for(int j = 0; j < 8; j++){

			dx = HGCreateTable[j][0];
			dy = HGCreateTable[j][1];
			dz = HGCreateTable[j][2];

			//     for(dx = -1; dx < 1; dx++)
			// 	 for(dy = -1; dy < 1; dy++)
			// 	   for(dz = -1; dz < 1; dz++){

			//dx = dy = dz = 0;

			hash_value = hashValue(index_x + dx,
					index_y + dy,
					index_z + dz);

			it = t_cells.find(hash_value);
			if(it == t_cells.end()){
				TetraederBox* new_box = new TetraederBox(Vertex((index_x + dx) * voxelsize + xmin,
						                                        (index_y + dy) * voxelsize + ymin,
						                                        (index_z + dz) * voxelsize + zmin), voxelsize);
				t_cells[hash_value] = new_box;
			}
		}
	}
  cout << "##### Finished MT-Grid Creation. Number of generated cells: " << cells.size() << endl;
}

void HashGrid::createMesh(){

  cout << "##### Creating Mesh..." << endl;

  hash_map<int, Box*>::iterator it;
  Box* b;
  int global_index = 0;
  int c = 0;

  for(it = cells.begin(); it != cells.end(); it++){
    if(c % 1000 == 0) cout << "##### Iterating Cells... " << c << " / " << cells.size() << endl;;
    b = it->second;
    global_index = b->getApproximation(global_index,
							    mesh,
							    interpolator);
    c++;
  }

  mesh.printStats();
  mesh.finalize();

}

void HashGrid::createMeshMT(){

  cout << "##### Creating Mesh..." << endl;

  hash_map<int, TetraederBox*>::iterator it;
  TetraederBox* b;
  int global_index = 0;
  int c = 0;

  for(it = t_cells.begin(); it != t_cells.end(); it++){
    if(c % 1000 == 0) cout << "##### Iterating Cells... " << c << " / " << t_cells.size() << endl;;
    b = it->second;
    global_index = b->getApproximation(global_index,
							    mesh,
							    interpolator);
    c++;
  }

//  cout << "##### Finished MT-Mesh Generation..." << endl;
//  cout << "##### Current Mesh has " << mesh.indices.size() / 3
//	  << " faces using " << mesh.vertices.size() << " vertices." << endl;

}


int HashGrid::readPoints(string filename, float scale){

  ifstream in(filename.c_str());

  //Vector to tmp-store points in file
  vector<BaseVertex> pts;

  //Read all points. Save maximum and minimum dimensions and
  //calculate maximum indices.
  int c = 0;

  //Get number of data fields to ignore
  int number_of_dummys = getFieldsPerLine(filename) - 3;

  //Point coordinates
  float x, y, z, dummy;

  //Read file
  while(in.good()){
    in >> x >> y >> z;
    for(int i = 0; i < number_of_dummys; i++){
	 in >> dummy;
    }

    x *= scale;
    y *= scale;
    z *= scale;

    xmin = min(xmin, x);
    ymin = min(ymin, y);
    zmin = min(zmin, z);

    xmax = max(xmax, x);
    ymax = max(ymax, y);
    zmax = max(zmax, z);

    pts.push_back(BaseVertex(x,y,z));

    c++;

    if(c % 10000 == 0) cout << "##### Reading Points... " << c << endl;
  }

  cout << "##### Finished Reading. Number of Data Points: " << pts.size() << endl;


  //Calculate bounding box
  float x_size = fabs(xmax - xmin);
  float y_size = fabs(ymax - ymin);
  float z_size = fabs(zmax - zmin);

  float max_size = max(max(x_size, y_size), z_size);

  //Save needed grid parameters
  max_index = (int)ceil( (max_size + 5 * voxelsize) / voxelsize);
  max_index_square = max_index * max_index;

  max_index_x = (int)ceil(x_size / voxelsize) + 1;
  max_index_y = (int)ceil(y_size / voxelsize) + 2;
  max_index_z = (int)ceil(z_size / voxelsize) + 3;

  //Create ANNPointArray
  cout << "##### Creating ANN Points " << endl;
  points = annAllocPts(c, 3);

  for(size_t i = 0; i < pts.size(); i++){
    points[i][0] = pts[i].x;
    points[i][1] = pts[i].y;
    points[i][2] = pts[i].z;
  }

  pts.clear();

  return c;
}

void HashGrid::writeMesh(){

//  cout << "##### Writing 'mesh.ply'" << endl;
//
//  PLYWriter w("mesh.ply");
//
//  w.writeHeader();
//  w.addMesh(mesh);
//  w.writeFooter();

}

void HashGrid::writeBorders(){

//  cout << "##### Writing 'mesh.bor'" << endl;
//
//  ofstream out("mesh.bor");
//
//  for(size_t i = 0; i < mesh.indices.size() - 3; i+= 3){
//    BaseVertex v1 = mesh.vertices[i];
//    BaseVertex v2 = mesh.vertices[i+1];
//    BaseVertex v3 = mesh.vertices[i+2];
//    out << "BEGIN" << endl;
//    out << v1.x << " " << v1.y << " " << v1.z << endl;
//    out << v2.x << " " << v2.y << " " << v2.z << endl;
//    out << v3.x << " " << v3.y << " " << v3.z << endl;
//    out << "END" << endl;
//  }
//
//  out.close();
}

void HashGrid::writeGrid(){

  cout << "##### Writing 'grid.hg'" << endl;

  ofstream out("grid.hg");

  out << number_of_points << endl;

  for(int i = 0; i < number_of_points; i++){
    out << points[i][0] << " " <<  points[i][1] << " " << points[i][2] << endl;
  }

  ColorVertex corners[8];
  hash_map<int, Box*>::iterator it;
  Box* box;
  uchar r, g, b;

  for(it = cells.begin(); it != cells.end(); it++){
    box = it->second;
    box->getCorners(corners);
    for(int i = 0; i < 8; i++){
	 r = g = b = 0.0;
	 if(box->configuration[i]) r = 255; else b = 255;
	 out << corners[i].x << " " << corners[i].y << " " << corners[i].z << " "
		<< r << " " << g << " " << b << endl;
    }
  }

}

int HashGrid::getFieldsPerLine(string filename){

  ifstream in(filename.c_str());

  //Get first line from file
  char first_line[1024];
  in.getline(first_line, 1024);
  in.close();

  //Get number of blanks
  int c = 0;
  char* pch = strtok(first_line, " ");
  while(pch != NULL){
    c++;
    pch = strtok(NULL, " ");
  }

  in.close();

  return c;
}
