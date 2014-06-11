/**
 * @file HeightMap.hpp
 * @date 2012
 * @author Johan Klokkhammer Helsing
 */

#ifndef HEIGHTMAP_HPP
#define HEIGHTMAP_HPP

#include "hmath.hpp"
#include <ostream>
#include <vector>

namespace helsing {

/** @brief A HeightMap is a two dimensional grid of float values
 *
 */
class HeightMap {
public:
	/// @param size width and height of the heightmap
	explicit HeightMap(unsigned int size);

	/// Initialize a heightmap by copying an array of floats
	explicit HeightMap(const float heights[], const unsigned int size);

	virtual ~HeightMap();

	/// @return the width/height of the heightmap
	virtual unsigned int getSize() const {return size;}

	/// @return normal of the gridpoints, assuming distance between gridpoints is 1
	virtual const helsing::Vec4 getNormal(int x, int z) const;

	/// @return position of the grid point, assuming distance between gridpoints is 1
	virtual const helsing::Vec4 getPoint(unsigned int x, unsigned int z) const;

	/// @return height at the given grid coordinate
	virtual float getHeight(unsigned int x, unsigned int y) const {return heights[x+y*size];}

	/// Sets the height at the given grid coordinate
	virtual void setHeight(unsigned int x, unsigned int y, float h){heights[x+y*size] = h;}

	// changes the height at the given grid coordinate according to h (height += h)
	virtual void addToHeight(unsigned int x, unsigned int y, float h){heights[x+y*size] += h;}

	/// Sets all the heights to zero
	virtual void flatten();

	HeightMap(const HeightMap&);
	HeightMap & operator=(const HeightMap&);

private:
	std::vector<float> heights;
	unsigned int size;
};

inline std::ostream& operator<< (std::ostream &out, const HeightMap& heightMap){
	//TODO restore these afterwards
	out.setf(std::ios::fixed);
	out.setf(std::ios::showpoint);
	out.precision(2); //to limit to 2 decimal places

	for(unsigned int y=0;y<heightMap.getSize(); ++y){
		for(unsigned int x=0; x<heightMap.getSize(); ++x){
			out << heightMap.getHeight(x,y) << " ";
		}
		out << "\n";
	}
	return out;
}

}

#endif // HEIGHTMAP_HPP
