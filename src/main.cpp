
#include <iostream>
#include <mpi.h>
#include "composite.h"
#include "cell.h"
#include "cellularSpace.h"

using  namespace std;

typedef pair<int, int> CSDim;

class MPICell : public Cell, public CompositeInterface< vectorComposite<double> >{
public:
   MPICell(){}

   MPICell(const MPICell& c){
      if(this != &c){
         *this = c;
      }
   }

   ~MPICell(){}

   MPICell& operator=(const MPICell&){
      return *this;
   }
};

template <class Index>
class MPIRegion : public CompositeInterface< multimapComposite<Index, MPICell*> >{
public:
   void add(Index index, MPICell* cell){
      pair<Index, MPICell*>  indexCellPair;

      indexCellPair.first = index;
      indexCellPair.second = cell;

      CompositeInterface< multimapComposite<Index, MPICell*> >::add(indexCellPair);
   }

   MPICell* operator [](Index index){
      pair<Index, MPICell*>  indexCellPair;

      indexCellPair =
           CompositeInterface< multimapComposite<Index, MPICell*> >::operator [](index);

      return indexCellPair.second;
   }
};

class MPICellularSpace : public MPIRegion<CellIndex>{
public:
   MPICellularSpace(){}

   MPICellularSpace(const MPICellularSpace& cs){
      if(this != &cs){
         *this = cs;
      }
   }

   ~MPICellularSpace(){}

   MPICellularSpace& operator=(const MPICellularSpace&){
      return *this;
   }

   void setDimensions(const CSDim& dim){
      this->dimensions = dim;
   }

   CSDim getDimension(){
      return this->dimensions;
   }

private:
   CSDim dimensions;
};

class MPIImpl : public Implementation{
public:
   MPIImpl(){}

   MPIImpl(const MPIImpl& i){
      if(this != &i){
         *this = i;
      }
   }

   ~MPIImpl(){}

   MPIImpl& operator=(const MPIImpl& i){
      if(this != &i){
         *this = i;
      }
      return *this;
   }
};

typedef Interface<MPIImpl> MPIInterf;

class MPISec : public MPIInterf{
public:
   void lines(const int& r, const CSDim& dim){
      this->rank = r;
      this->cellularSpace = new MPICellularSpace();
      this->cellularSpace->setDimensions(dim);

      CellIndex index;
      for(int i = 0; i < dim.first; i++){
         for(int j = 0; j < dim.second; j++){
            index.first = i;
            index.second = j;

            this->cellularSpace->add(index, new MPICell());
         }
      }
   }

private:
   int rank;
   MPICellularSpace* cellularSpace;
};

class MPIPrim : public MPIInterf{
public:
   MPIPrim(const MPI_Comm& c):comm(c){}

   void lines(int& n_sec){
      CSDim temp = this->dim;
      temp.first = temp.first/n_sec;

      for(int i = 1; i <= n_sec; i++){
         this->sec.add(new MPISec());
         this->sec[i]->lines(i, temp);
      }
   }

   void setComm(MPI_Comm& c){
      this->comm = c;
   }

   void setDimensions(CSDim& d){
      this->dim = d;
   }

private:
   MPI_Comm comm;
   CSDim dim;
   CompositeInterface< vectorComposite<MPISec*> > sec;
};

int main(int argc, char *argv[]){

   // MPICell c1 = MPICell();
   // c1.add(10.5);
   // c1.add(3.3);
   //
   // MPISec* sec1 = new MPISec();
   // CSDim dim; dim.first = 10; dim.second = 5;
   // sec1->lines(1, dim);

   int size;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &size);

   CSDim dim; dim.first = 10; dim.second = 5;

   MPIPrim* primary = new MPIPrim(MPI_COMM_WORLD);
   primary->setDimensions(dim);

   MPI_Finalize();

   return 0;
}
