
#include <iostream>
#include "cell.h"
#include "cellularSpace.h"
#include <mpi.h>

using namespace std;

class MyCellularSpace : public CellularSpace {
public:
	void forEach(Event &event, Agent *agent){
		Region_<CellIndex>::iterator it = Region_<CellIndex>::pImpl_->begin();

		while(it != Region_<CellIndex>::pImpl_->end()){
			it->second->execute(event, agent);
			it++;
		}
	}

	void forEach(void (*function)(Cell *)){
		Region_<CellIndex>::iterator it = Region_<CellIndex>::pImpl_->begin();

		while(it != Region_<CellIndex>::pImpl_->end()){
			function(it->second);
			it++;
		}
	}
};

typedef int CellularSpaceIndex;
typedef pair<int, int> CellularSpaceSize;

template <class Index>
class MyRegion : public CompositeInterface< multimapComposite <Index, CellularSpace *> > {
public:

	// add a CellularSpace to the regions
	void add(Index index, CellularSpace *cellularSpace) {
		pair<Index, CellularSpace *> indexCellularSpacePair;

		indexCellularSpacePair.first = index;
		indexCellularSpacePair.second = cellularSpace;

		CompositeInterface< multimapComposite< Index, CellularSpace *> >::add(indexCellularSpacePair);
	}

	// return CellularSpace pointer if it exist, otherwise return NULL
	CellularSpace* operator [](Index index){
		pair<Index, CellularSpace *> indexCellularSpacePair
													= CompositeInterface< multimapComposite <Index, CellularSpace *> >::operator [](index);

		return indexCellularSpacePair.second;
	}

	// return the composite size
	int size(){ return (CompositeInterface< multimapComposite< Index, CellularSpace *> >::pImpl_)->size();}

	void forEach(void (*function)(Event &, Agent *), Event &event, Agent *agent){
		MyRegion<CellularSpaceIndex>::iterator it = MyRegion<CellularSpaceIndex>::pImpl_->begin();

		while(it != MyRegion<CellularSpaceIndex>::pImpl_->end()){
			// error: ‘class CellularSpace’ has no member named ‘forEach’
			// it->second->forEach(function, event, agent);
			it++;
		}
	}
};

class MyEnvironmentImpl : public Implementation {
public:
};

typedef Interface<MyEnvironmentImpl> MyEnvironmentInterf;

class MyEnvironment : public MyEnvironmentInterf,
							 public MyRegion<int>{
public:
	MyEnvironment(){ }

	void init(int argc, char *argv[], const MPI_Comm &comm){
		this->comm = comm;
		MPI_Init(&argc, &argv);
	}

	void finalize(){
		MPI_Finalize();
	}

	void lineAlloc(CellularSpaceSize size){
		MPI_Comm_size(comm, &comm_size);
		MPI_Comm_rank(comm, &comm_rank);

		int height = size.first/comm_size;
		CellIndex ci;

		for(int proc = 0; proc < comm_size; proc++){
			if(proc == comm_rank){
				CellularSpace *cs = new CellularSpace();
				for(int i = 0; i < height; i++){
					for(int j = 0; j < size.second; j++){
						ci.first = i;
						ci.second = j;
						cs->add(ci, new Cell());
					}
				}

				this->add(proc, new CellularSpace());

				delete cs;
			}
		}
	}

private:
	MPI_Comm comm;
	int comm_rank;
	int comm_size;
};

int main(int argc, char *argv[]) {

	// instancias de Cell
	Cell *c1 = new Cell();
	Cell *c2 = new Cell();
	Cell *c3 = new Cell();
	// CellImpl *c2 = c1->clone();
	// NeighCmpstInterf& nci = c1->getNeighborhoods();
	CellIndex c1i, c2i, c3i;

	c1i.first = 0;	c1i.second = 0;
	c2i.first = 1;	c2i.second = 0;
	c3i.first = 2;	c3i.second = 0;

	// instancias de CellularSpace
	CellularSpace *cs1 = new CellularSpace();
	cs1->add(c1i, c1);
	CellularSpace *cs2 = new CellularSpace();
	cs2->add(c2i, c2);
	CellularSpace *cs3 = new CellularSpace();
	cs2->add(c3i, c3);

	MyRegion<int> *mr = new MyRegion<int>();
	mr->add(0, cs1);
	mr->add(1, cs2);
	mr->add(2, cs3);
	cout << mr->size() << endl;

	// cs1->forEach();
	// cout << endl;
	// DistributedCellularSpace *dcs1 = new DistributedCellularSpace();
	// dcs1->add(c1i, c1);
	// dcs1->forEach(&fun, '|');
	// cout << endl;

	delete cs1;	delete cs2;	delete cs3;
	delete c1;	delete c2;	delete c3;

	MyEnvironment *env = new MyEnvironment();
	env->init(argc, argv, MPI_COMM_WORLD);

	CellularSpaceSize cssize;
	cssize.first = 3;
	cssize.second = 3;

	env->lineAlloc(cssize);
	env->finalize();

	return 0;
}
