#ifndef SPATIAL_HASHMAP_H
#define SPATIAL_HASHMAP_H

#include <unordered_map>
#include <QPoint>
#include <math.h>
#include <boost/functional/hash.hpp>
#include <exception>

struct BoundingBox{
    QPoint min, max;
};

namespace std {
  template <>
  struct hash<QPoint>
  {
    std::size_t operator()(const QPoint & t) const
    {
      std::size_t val = 0 ;
      boost::hash_combine(val,t.x());
      boost::hash_combine(val,t.y());
      return val;
    }
  };
}

template <class T> class SpatialHashMap : private std::unordered_map<QPoint, T>{
public:
    class OutOfRangeException : public std::exception
    {
    public:
        virtual const char* what() const noexcept
        {
            return "Query out of spatial hashmap range!";
        }
    };
    class UninitialisedException : public std::exception
    {
    public:
        virtual const char* what() const noexcept
        {
            return "Queried an unitialised cell!";
        }
    };

    enum Space{
        _HASHMAP,
        _WORLD
    };

    SpatialHashMap(int cell_width, int cell_height, int n_horizontal_cells, int n_vertical_cells) :
        m_cell_width(cell_width), m_cell_height(cell_height),
        m_horizontal_cell_count(n_horizontal_cells),
        m_vertical_cell_count(n_vertical_cells)
    {

    }

    int getHorizontalCellCount() const { return m_horizontal_cell_count; }
    int getVerticalCellCount() const { return m_vertical_cell_count; }
    int getCellWidth() const { return m_cell_width; }
    int getCellHeight() const { return m_cell_height; }

    std::vector<QPoint> getPoints(QPoint p_center, float p_radius, bool center_check) const
    {
        // First calculate the bounding box
        BoundingBox bb(SpatialHashMap::get_bounding_box(p_center, p_radius));

        std::vector<QPoint> ret;
        for(int x (bb.min.x()); x < std::min(m_horizontal_cell_count, bb.max.x()+1); x++)
        {
            for(int y (bb.min.y()); y < std::min(m_vertical_cell_count, bb.max.y()+1); y++)
            {
                // Ensure at least the center of the cell is within reach
                if(!center_check || (pow(((x*m_cell_width) + m_cell_width/2) - p_center.x(),2) +
                      pow(((y*m_cell_height) + m_cell_height/2) - p_center.y(),2) < p_radius*p_radius))
                {
                    ret.push_back(QPoint(x,y));
                }
            }
        }

        // Too small, at least add the single cell in which it resides
        if(ret.size() == 0)
        {
            QPoint local_space(toHashMapCoordinates(p_center));
            if(local_space.x() < m_horizontal_cell_count && local_space.y() < m_vertical_cell_count)
                ret.push_back(local_space);
        }

        return ret;
    }

    T& getCell(QPoint center, Space space)
    {
        if(space == Space::_WORLD)
            center = toHashMapCoordinates(center);

        if(center.x() < m_horizontal_cell_count && center.y() < m_vertical_cell_count)
        {
            if(m_initialised_cells.find(center) == m_initialised_cells.end())
                init_cell(center);

            return std::unordered_map<QPoint, T>::find(center)->second;
        }
        throw OutOfRangeException();
    }

    const T& getCell(QPoint center, Space space) const
    {
        if(space == Space::_WORLD)
            center = toHashMapCoordinates(center);

        if(center.x() < m_horizontal_cell_count && center.y() < m_vertical_cell_count)
        {
            if(m_initialised_cells.find(center) == m_initialised_cells.end())
                    throw UninitialisedException();

            return std::unordered_map<QPoint, T>::find(center)->second;
        }
        throw OutOfRangeException();
    }

//    T& operator[](const QPoint & key, bool init_if_necessary = true)
//    {
//        if(key.x() < m_horizontal_cell_count && key.y() < m_vertical_cell_count)
//        {
//            if(m_initialised_cells.find(key) == m_initialised_cells.end())
//            {
//                if(init_if_necessary)
//                    init_cell(key);
//                else
//                    throw UninitialisedException();
//            }

//            return std::unordered_map<QPoint, T>::find(key)->second;
//        }

//        throw OutOfRangeException();
//    }

//    const T& operator[](const QPoint & key, bool init_if_necessary = true)
//    {
//        if(key.x() < m_horizontal_cell_count && key.y() < m_vertical_cell_count)
//        {
//            if(m_initialised_cells.find(key) == m_initialised_cells.end())
//            {
//                if(init_if_necessary)
//                    init_cell(key);
//                else
//                    throw UninitialisedException();
//            }

//            return std::unordered_map<QPoint, T>::find(key)->second;
//        }

//        throw OutOfRangeException();
//    }

//    const T& operator[](const QPoint & key) const
//    {
//        if(key.x() < m_horizontal_cell_count && key.y() < m_vertical_cell_count)
//        {
//            if(m_initialised_cells.find(key) != m_initialised_cells.end())
//                return std::unordered_map<QPoint, T>::find(key)->second;

//            throw UninitialisedException();
//        }

//        throw OutOfRangeException();
//    }

    bool coversMultipleCells(const QPoint & center, int radius) const
    {
        BoundingBox bb(get_bounding_box(center, radius));
        return (bb.min != bb.max);
    }

    typename std::unordered_map<QPoint, T>::iterator begin()
    {
        return std::unordered_map<QPoint, T>::begin();
    }

    typename std::unordered_map<QPoint, T>::iterator end()
    {
        return std::unordered_map<QPoint, T>::end();
    }

    typename std::unordered_map<QPoint, T>::const_iterator cbegin() const
    {
        return std::unordered_map<QPoint, T>::cbegin();
    }

    typename std::unordered_map<QPoint, T>::const_iterator cend() const
    {
        return std::unordered_map<QPoint, T>::cend();
    }

    void clear()
    {
        std::unordered_map<QPoint, T>::clear();
        m_initialised_cells.clear();
    }

    int getCellArea() const
    {
        return m_cell_height*m_cell_width;
    }

    int size() const
    {
        return std::unordered_map<QPoint, T>::size();
    }

    bool initialised(const QPoint & cell) const
    {
        return m_initialised_cells.find(cell) != m_initialised_cells.end();
    }

    bool ws_initialised(const QPoint & ws_coord) const
    {
        return initialised(toHashMapCoordinates(ws_coord));
    }

protected:
    BoundingBox get_bounding_box(const QPoint & p_center, int p_radius) const
    {
        // First calculate the bounding box
        float x_min(p_center.x()-p_radius);
        float x_max(p_center.x()+p_radius);

        float y_min(p_center.y()-p_radius);
        float y_max(p_center.y()+p_radius);

        BoundingBox bb;
        QPoint mins(std::max(0.0f, x_min), std::max(0.0f, y_min));
        QPoint maxs(std::min(((float) getHorizontalCellCount()-1) * getCellWidth(), x_max), std::min(((float) getVerticalCellCount()-1) * getCellHeight(), y_max) );
        bb.min = toHashMapCoordinates(mins);
        bb.max = toHashMapCoordinates(maxs);

        return bb;
    }

    QPoint toHashMapCoordinates( const QPoint & coord ) const
    {
        QPoint ret(coord.x() / m_cell_width, coord.y() / m_cell_height);
        if(ret.x() < m_horizontal_cell_count && ret.y() < m_vertical_cell_count)
        {
            return ret;
        }
        throw OutOfRangeException();
    }

private:
    void init_cell(const QPoint & key)
    {
        std::unordered_map<QPoint, T>::emplace(key, T());
        m_initialised_cells.emplace(key, true);
    }

    std::unordered_map<QPoint, bool> m_initialised_cells;
    int m_cell_width, m_cell_height, m_horizontal_cell_count, m_vertical_cell_count;
};

#endif //SPATIAL_HASHMAP_H
