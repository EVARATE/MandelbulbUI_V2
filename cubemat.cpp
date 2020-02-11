#include "cubemat.h"

void cubeMat::initCubeMat(){
    //cubeMat3d as big as bCloud
    cubeSlots.resize(bCloud.xsize);
    for(int i = 0; i < bCloud.xsize; ++i){
        cubeSlots[i].resize(bCloud.ysize);
        for(int j = 0; j < bCloud.ysize; ++j){
            cubeSlots[i][j].resize(bCloud.zsize);
        }
    }
    placeCubes();
    isInit = true;
}
void cubeMat::placeCubes(){
    for(int i = 0; i < cubeSlots.size(); ++i){
    for(int j = 0; j < cubeSlots[i].size(); ++j){
    for(int k = 0; k < cubeSlots[i][j].size(); ++k){
        ivec index = {i,j,k};
        if(bCloud.getHState(index)){
            cube localCube;
            genLocalCube(index,localCube);
            placeCubeAt(index, localCube);
        }else{
            cube localCube(6, emptyFace);//Empty cube
            placeCubeAt(index, localCube);
        }
    }
    }
    }
}
void cubeMat::placeCubeAt(ivec& index, cube& localCube){
    cubeSlots[index[0]][index[1]][index[2]] = localCube;
}
void cubeMat::genLocalVerts(ivec& index, std::vector<ivec>& localVerts){
    localVerts.resize(8);
    localVerts[0] = {index[0]  ,index[1]  ,index[2]  };
    localVerts[1] = {index[0]+1,index[1]  ,index[2]  };
    localVerts[2] = {index[0]+1,index[1]+1,index[2]  };
    localVerts[3] = {index[0]  ,index[1]+1,index[2]  };
    localVerts[4] = {index[0]  ,index[1]  ,index[2]+1};
    localVerts[5] = {index[0]+1,index[1]  ,index[2]+1};
    localVerts[6] = {index[0]+1,index[1]+1,index[2]+1};
    localVerts[7] = {index[0]  ,index[1]+1,index[2]+1};
}
void cubeMat::genLocalCube(ivec& index, cube& localCube){
    std::vector<ivec> lVerts;
    genLocalVerts(index, lVerts);
    localCube.resize(6);
    localCube[0] ={lVerts[0],lVerts[3],lVerts[7],lVerts[4]};//0374 Left
    localCube[1] ={lVerts[0],lVerts[1],lVerts[5],lVerts[4]};//0154 Back
    localCube[2] ={lVerts[1],lVerts[2],lVerts[6],lVerts[5]};//1265 Right
    localCube[3] ={lVerts[3],lVerts[2],lVerts[6],lVerts[7]};//3267 Front
    localCube[4] ={lVerts[4],lVerts[5],lVerts[6],lVerts[7]};//4567 Up
    localCube[5] ={lVerts[0],lVerts[1],lVerts[2],lVerts[3]};//0123 Down
    //The order is chosen so that the vertices of two parallel
    //faces in a cube are always connected (if that makes sense)
}

void cubeMat::remOverlapping(cube &cubeA, cube &cubeB){
    //If cubes are next to eachother, these index pairs of their faces can overlap:
    //LR:02 ; FB:13 ; UD:45 (in either order)
    if(cubeA[0] == cubeB[2]){
        cubeA[0] = emptyFace;
        cubeB[2] = emptyFace;
    }
    else if(cubeA[1] == cubeB[3]){
        cubeA[1] = emptyFace;
        cubeB[3] = emptyFace;
    }
    else if(cubeA[4] == cubeB[5]){
        cubeA[4] = emptyFace;
        cubeB[5] = emptyFace;
    }
    else if(cubeA[2] == cubeB[0]){
        cubeA[2] = emptyFace;
        cubeB[0] = emptyFace;
    }
    else if(cubeA[3] == cubeB[1]){
        cubeA[3] = emptyFace;
        cubeB[1] = emptyFace;
    }
    else if(cubeA[5] == cubeB[4]){
        cubeA[5] = emptyFace;
        cubeB[4] = emptyFace;
    }

}
void cubeMat::remAllOverlapping(){
    if(!isInit){return;}
    for(int i = 0; i < cubeSlots.size(); ++i){
    for(int j = 0; j < cubeSlots[i].size(); ++j){
    for(int k = 0; k < cubeSlots[i][j].size(); ++k){
        cube active = cubeSlots[i][j][k];
        if(!isEmpty(active)){
            remOverlapping(cubeSlots[i-1][j][k],active);
            remOverlapping(cubeSlots[i+1][j][k],active);
            remOverlapping(cubeSlots[i][j-1][k],active);
            remOverlapping(cubeSlots[i][j+1][k],active);
            remOverlapping(cubeSlots[i][j][k-1],active);
            remOverlapping(cubeSlots[i][j][k+1],active);
        }
    }
    }
    }

}
void cubeMat::saveObj(std::string fileName){
    std::ofstream ofile;
    ofile.open(fileName);
    if(!ofile.is_open()){return;}
    std::string polyBuffer;
    int vCounter = 0;
    ofile << "#DANGER: File generated via MandelbulbUI_V2:\n";
    ofile << "o MandelbulbUI_V2\n";
    for(int i = 0; i < cubeSlots.size(); ++i){
    for(int j = 0; j < cubeSlots[i].size(); ++j){
    for(int k = 0; k < cubeSlots[i][j].size(); ++k){
        if(!isEmpty(cubeSlots[i][j][k])){
            ivec index = {i,j,k};
            dvec coord(3);
            bCloud.convIndexToCoord(index,coord);

            ofile << "v " << cubeSlots[i][j][k][0][0][0] << " " << cubeSlots[i][j][k][0][0][1] << " " << cubeSlots[i][j][k][0][0][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][0][1][0] << " " << cubeSlots[i][j][k][0][1][1] << " " << cubeSlots[i][j][k][0][1][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][0][2][0] << " " << cubeSlots[i][j][k][0][2][1] << " " << cubeSlots[i][j][k][0][2][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][0][3][0] << " " << cubeSlots[i][j][k][0][3][1] << " " << cubeSlots[i][j][k][0][3][2] << "\n";
            vCounter++;  polyBuffer.append("f " + std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + "\n");
            ofile << "v " << cubeSlots[i][j][k][1][0][0] << " " << cubeSlots[i][j][k][1][0][1] << " " << cubeSlots[i][j][k][1][0][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][1][1][0] << " " << cubeSlots[i][j][k][1][1][1] << " " << cubeSlots[i][j][k][1][1][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][1][2][0] << " " << cubeSlots[i][j][k][1][2][1] << " " << cubeSlots[i][j][k][1][2][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][1][3][0] << " " << cubeSlots[i][j][k][1][3][1] << " " << cubeSlots[i][j][k][1][3][2] << "\n";
            vCounter++;  polyBuffer.append("f " + std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + "\n");
            ofile << "v " << cubeSlots[i][j][k][2][0][0] << " " << cubeSlots[i][j][k][2][0][1] << " " << cubeSlots[i][j][k][2][0][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][2][1][0] << " " << cubeSlots[i][j][k][2][1][1] << " " << cubeSlots[i][j][k][2][1][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][2][2][0] << " " << cubeSlots[i][j][k][2][2][1] << " " << cubeSlots[i][j][k][2][2][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][2][3][0] << " " << cubeSlots[i][j][k][2][3][1] << " " << cubeSlots[i][j][k][2][3][2] << "\n";
            vCounter++;  polyBuffer.append("f " + std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + "\n");
            ofile << "v " << cubeSlots[i][j][k][3][0][0] << " " << cubeSlots[i][j][k][3][0][1] << " " << cubeSlots[i][j][k][3][0][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][3][1][0] << " " << cubeSlots[i][j][k][3][1][1] << " " << cubeSlots[i][j][k][3][1][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][3][2][0] << " " << cubeSlots[i][j][k][3][2][1] << " " << cubeSlots[i][j][k][3][2][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][3][3][0] << " " << cubeSlots[i][j][k][3][3][1] << " " << cubeSlots[i][j][k][3][3][2] << "\n";
            vCounter++;  polyBuffer.append("f " + std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + "\n");
            ofile << "v " << cubeSlots[i][j][k][4][0][0] << " " << cubeSlots[i][j][k][4][0][1] << " " << cubeSlots[i][j][k][4][0][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][4][1][0] << " " << cubeSlots[i][j][k][4][1][1] << " " << cubeSlots[i][j][k][4][1][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][4][2][0] << " " << cubeSlots[i][j][k][4][2][1] << " " << cubeSlots[i][j][k][4][2][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][4][3][0] << " " << cubeSlots[i][j][k][4][3][1] << " " << cubeSlots[i][j][k][4][3][2] << "\n";
            vCounter++;  polyBuffer.append("f " + std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + "\n");
            ofile << "v " << cubeSlots[i][j][k][5][0][0] << " " << cubeSlots[i][j][k][5][0][1] << " " << cubeSlots[i][j][k][5][0][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][5][1][0] << " " << cubeSlots[i][j][k][5][1][1] << " " << cubeSlots[i][j][k][5][1][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][5][2][0] << " " << cubeSlots[i][j][k][5][2][1] << " " << cubeSlots[i][j][k][5][2][2] << "\n";
            ofile << "v " << cubeSlots[i][j][k][5][3][0] << " " << cubeSlots[i][j][k][5][3][1] << " " << cubeSlots[i][j][k][5][3][2] << "\n";
            vCounter++;  polyBuffer.append("f " + std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + " ");
            vCounter++;  polyBuffer.append(std::to_string(vCounter) + "\n");

        }
    }
    }
    }
    ofile << polyBuffer;
    ofile.close();
}

bool cubeMat::isEmpty(cube& checkCube){
    if(checkCube == emptyCube){return true;}
    else{return false;}
}