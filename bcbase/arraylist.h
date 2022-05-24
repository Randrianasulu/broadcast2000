#ifndef ARRAYLIST_H
#define ARRAYLIST_H

// designed for lists of track numbers

template<class TYPE>
class ArrayList
{
public:
	ArrayList();
	virtual ~ArrayList();

	TYPE append(TYPE value);
	TYPE append();

	int remove();          // remove from end
	int remove(TYPE value);       // remove item containing
	int remove_number(int number);     // remove item numbered
	int remove_all();

	int sort();

	TYPE* values;
	long total;

private:
	long available;
};

template<class TYPE>
ArrayList<TYPE>::ArrayList()
{
	total = 0;
	available = 1;
	values = new TYPE[available];
}


template<class TYPE>
ArrayList<TYPE>::~ArrayList()
{
	delete [] values;
}

template<class TYPE>
TYPE ArrayList<TYPE>::append(TYPE value)            // add to end of list
{
	if(total + 1 > available) 
	{
		available *= 2;
		TYPE* newvalues = new TYPE[available];
		for(int i = 0; i < total; i++) newvalues[i] = values[i];
		delete values;
		values = newvalues;
	}
	
	values[total++] = value;
	return value;
}

template<class TYPE>
TYPE ArrayList<TYPE>::append()            // add to end of list
{
	if(total + 1 > available)
	{
		available *= 2;
		TYPE* newvalues = new TYPE[available];
		for(int i = 0; i < total; i++) newvalues[i] = values[i];
		delete values;
		values = newvalues;
	}
	total++;

	return values[total - 1];
}

template<class TYPE>
int ArrayList<TYPE>::remove(TYPE value)                   // remove value from anywhere in list
{
	int in, out;

	for(in = 0, out = 0; in < total;)
	{
		if(values[in] != value) values[out++] = values[in++];
		else 
		{ 
			in++; 
		}
	}
	total = out;
	return 0;
}

template<class TYPE>
int ArrayList<TYPE>::remove()
{
	total--;
	return 0;
}

template<class TYPE>
int ArrayList<TYPE>::remove_number(int number)                   // remove value from anywhere in list
{
	static int in, out;
	
	for(in = 0, out = 0; in < total;)
	{
		if(in != number) values[out++] = values[in++];
		else  in++;       // need to delete it here
	}
	total = out;
	return 0;
}

template<class TYPE>
int ArrayList<TYPE>::remove_all()
{
	total = 0;
	return 0;
}

template<class TYPE>
int ArrayList<TYPE>::sort()                    // sort from least to greatest value
{
	int result = 1;
	TYPE temp;

	while(result)
	{
		result = 0;
		for(int i = 0, j = 1; j < total; i++, j++)
		{
			if(values[j] < values[i])
			{
				temp = values[i];
				values[i] = values[j];
				values[j] = temp;
				result = 1;
			}
		}
	}
	return 0;
}

#endif
