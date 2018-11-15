This is the central library of team Matthias/Magdalena providing access to
- BftMaterials:  Constutitive Models compatible with
    - EdelweissFE
    - Abaqus (via Abaqus-UserLibraryInterface)
    - Plaxis (via PlaxisUserlibrary)
    - OpenSees (via OpenSeesBftMaterialWrapper)
    - mpFEM 
- BftUels: Finite Elements compatible with
    - EdelweissFE
    - Abaqus (via Abaqus-UserLibraryInterface)
    - mpFEM 
    
For compilation, please make sure that
- bftMechanics is present
- desired Materials and Elements are present
- a proper version of the Eigen library is at hand
- and CMakeLists.txt is configured

For extension of the library, please stick to the coding style by using 
clang-format with the provided .clang-format style file.