.. highlight:: c++

Containers
==========

This section describes all the containers provided by the library.
Check the :ref:`design` section for a general discussion of these
containers and how to use them.

box
---

.. doxygenclass:: immer::box
    :members:
    :undoc-members:

array
-----

.. doxygenclass:: immer::array
    :members:
    :undoc-members:

vector
------

.. doxygenclass:: immer::vector
    :members:
    :undoc-members:

flex_vector
-----------

.. doxygenclass:: immer::flex_vector
    :members:
    :undoc-members:

set
---

.. doxygenclass:: immer::set
    :members:
    :undoc-members:

map
---

.. doxygenclass:: immer::map
    :members:
    :undoc-members:

ordered_set
-----------

Immutable ordered set of values, implemented using a B+ tree. Provides efficient
insertion, deletion, and lookup while maintaining elements in sorted order.

.. doxygenclass:: immer::ordered_set
    :members:
    :undoc-members:

ordered_map
-----------

Immutable ordered mapping of keys to values, implemented using a B+ tree.
Provides efficient insertion, deletion, and lookup while maintaining key-value
pairs sorted by key.

.. doxygenclass:: immer::ordered_map
    :members:
    :undoc-members:

table
-----

.. doxygenclass:: immer::table
    :members:
    :undoc-members:
