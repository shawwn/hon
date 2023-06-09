/*
 * types.c: converter functions between the internal representation
 *          and the Python objects
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */
#include "libxml_wrap.h"

PyObject *
libxml_intWrap(int val)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_intWrap: val = %d\n", val);
#endif
    ret = PyInt_FromLong((long) val);
    return (ret);
}

PyObject *
libxml_longWrap(long val)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_longWrap: val = %ld\n", val);
#endif
    ret = PyInt_FromLong(val);
    return (ret);
}

PyObject *
libxml_doubleWrap(double val)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_doubleWrap: val = %f\n", val);
#endif
    ret = PyFloat_FromDouble((double) val);
    return (ret);
}

PyObject *
libxml_charPtrWrap(char *str)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlcharPtrWrap: str = %s\n", str);
#endif
    if (str == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    /* TODO: look at deallocation */
    ret = PyString_FromString(str);
    xmlFree(str);
    return (ret);
}

PyObject *
libxml_charPtrConstWrap(const char *str)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlcharPtrWrap: str = %s\n", str);
#endif
    if (str == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    /* TODO: look at deallocation */
    ret = PyString_FromString(str);
    return (ret);
}

PyObject *
libxml_xmlCharPtrWrap(xmlChar * str)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlCharPtrWrap: str = %s\n", str);
#endif
    if (str == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    /* TODO: look at deallocation */
    ret = PyString_FromString((char *) str);
    xmlFree(str);
    return (ret);
}

PyObject *
libxml_xmlCharPtrConstWrap(const xmlChar * str)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlCharPtrWrap: str = %s\n", str);
#endif
    if (str == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    /* TODO: look at deallocation */
    ret = PyString_FromString((char *) str);
    return (ret);
}

PyObject *
libxml_constcharPtrWrap(const char *str)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlcharPtrWrap: str = %s\n", str);
#endif
    if (str == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    /* TODO: look at deallocation */
    ret = PyString_FromString(str);
    return (ret);
}

PyObject *
libxml_constxmlCharPtrWrap(const xmlChar * str)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlCharPtrWrap: str = %s\n", str);
#endif
    if (str == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    /* TODO: look at deallocation */
    ret = PyString_FromString((char *) str);
    return (ret);
}

PyObject *
libxml_xmlDocPtrWrap(xmlDocPtr doc)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlDocPtrWrap: doc = %p\n", doc);
#endif
    if (doc == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    /* TODO: look at deallocation */
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) doc, (char *) "xmlDocPtr",
                                     NULL);
    return (ret);
}

PyObject *
libxml_xmlNodePtrWrap(xmlNodePtr node)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlNodePtrWrap: node = %p\n", node);
#endif
    if (node == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) node, (char *) "xmlNodePtr",
                                     NULL);
    return (ret);
}

PyObject *
libxml_xmlURIPtrWrap(xmlURIPtr uri)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlURIPtrWrap: uri = %p\n", uri);
#endif
    if (uri == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) uri, (char *) "xmlURIPtr",
                                     NULL);
    return (ret);
}

PyObject *
libxml_xmlNsPtrWrap(xmlNsPtr ns)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlNsPtrWrap: node = %p\n", ns);
#endif
    if (ns == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) ns, (char *) "xmlNsPtr",
                                     NULL);
    return (ret);
}

PyObject *
libxml_xmlAttrPtrWrap(xmlAttrPtr attr)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlAttrNodePtrWrap: attr = %p\n", attr);
#endif
    if (attr == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) attr, (char *) "xmlAttrPtr",
                                     NULL);
    return (ret);
}

PyObject *
libxml_xmlAttributePtrWrap(xmlAttributePtr attr)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlAttributePtrWrap: attr = %p\n", attr);
#endif
    if (attr == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) attr,
                                     (char *) "xmlAttributePtr", NULL);
    return (ret);
}

PyObject *
libxml_xmlElementPtrWrap(xmlElementPtr elem)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlElementNodePtrWrap: elem = %p\n", elem);
#endif
    if (elem == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) elem,
                                     (char *) "xmlElementPtr", NULL);
    return (ret);
}

PyObject *
libxml_xmlXPathContextPtrWrap(xmlXPathContextPtr ctxt)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlXPathContextPtrWrap: ctxt = %p\n", ctxt);
#endif
    if (ctxt == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) ctxt,
                                     (char *) "xmlXPathContextPtr", NULL);
    return (ret);
}

PyObject *
libxml_xmlXPathParserContextPtrWrap(xmlXPathParserContextPtr ctxt)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlXPathParserContextPtrWrap: ctxt = %p\n", ctxt);
#endif
    if (ctxt == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret = PyCObject_FromVoidPtrAndDesc((void *) ctxt,
                                       (char *) "xmlXPathParserContextPtr",
                                       NULL);
    return (ret);
}

PyObject *
libxml_xmlParserCtxtPtrWrap(xmlParserCtxtPtr ctxt)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlParserCtxtPtrWrap: ctxt = %p\n", ctxt);
#endif
    if (ctxt == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }

    ret =
        PyCObject_FromVoidPtrAndDesc((void *) ctxt,
                                     (char *) "xmlParserCtxtPtr", NULL);
    return (ret);
}

PyObject *
libxml_xmlXPathObjectPtrWrap(xmlXPathObjectPtr obj)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlXPathObjectPtrWrap: ctxt = %p\n", obj);
#endif
    if (obj == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    switch (obj->type) {
        case XPATH_XSLT_TREE: {
            if ((obj->nodesetval == NULL) ||
		(obj->nodesetval->nodeNr == 0) ||
		(obj->nodesetval->nodeTab == NULL)) {
                ret = PyList_New(0);
	    } else {
		int i, len = 0;
		xmlNodePtr node;

		node = obj->nodesetval->nodeTab[0]->children;
		while (node != NULL) {
		    len++;
		    node = node->next;
		}
		ret = PyList_New(len);
		node = obj->nodesetval->nodeTab[0]->children;
		for (i = 0;i < len;i++) {
                    PyList_SetItem(ret, i, libxml_xmlNodePtrWrap(node));
		    node = node->next;
		}
	    }
	    /*
	     * Return now, do not free the object passed down
	     */
	    return (ret);
	}
        case XPATH_NODESET:
            if ((obj->nodesetval == NULL)
                || (obj->nodesetval->nodeNr == 0)) {
                ret = PyList_New(0);
	    } else {
                int i;
                xmlNodePtr node;

                ret = PyList_New(obj->nodesetval->nodeNr);
                for (i = 0; i < obj->nodesetval->nodeNr; i++) {
                    node = obj->nodesetval->nodeTab[i];
                    /* TODO: try to cast directly to the proper node type */
                    PyList_SetItem(ret, i, libxml_xmlNodePtrWrap(node));
                }
            }
            break;
        case XPATH_BOOLEAN:
            ret = PyInt_FromLong((long) obj->boolval);
            break;
        case XPATH_NUMBER:
            ret = PyFloat_FromDouble(obj->floatval);
            break;
        case XPATH_STRING:
            ret = PyString_FromString((char *) obj->stringval);
            break;
        case XPATH_POINT:
        {
            PyObject *node;
            PyObject *indexIntoNode;
            PyObject *tuple;

            node = libxml_xmlNodePtrWrap(obj->user);
            indexIntoNode = PyInt_FromLong((long) obj->index);

            tuple = PyTuple_New(2);
            PyTuple_SetItem(tuple, 0, node);
            PyTuple_SetItem(tuple, 1, indexIntoNode);

            ret = tuple;
            break;
        }
        case XPATH_RANGE:
        {
            unsigned short bCollapsedRange;

            bCollapsedRange = ( (obj->user2 == NULL) ||
		                ((obj->user2 == obj->user) && (obj->index == obj->index2)) );
            if ( bCollapsedRange ) {
                PyObject *node;
                PyObject *indexIntoNode;
                PyObject *tuple;
                PyObject *list;

                list = PyList_New(1);

                node = libxml_xmlNodePtrWrap(obj->user);
                indexIntoNode = PyInt_FromLong((long) obj->index);

                tuple = PyTuple_New(2);
                PyTuple_SetItem(tuple, 0, node);
                PyTuple_SetItem(tuple, 1, indexIntoNode);

                PyList_SetItem(list, 0, tuple);

                ret = list;
            } else {
                PyObject *node;
                PyObject *indexIntoNode;
                PyObject *tuple;
                PyObject *list;

                list = PyList_New(2);

                node = libxml_xmlNodePtrWrap(obj->user);
                indexIntoNode = PyInt_FromLong((long) obj->index);

                tuple = PyTuple_New(2);
                PyTuple_SetItem(tuple, 0, node);
                PyTuple_SetItem(tuple, 1, indexIntoNode);

                PyList_SetItem(list, 0, tuple);

                node = libxml_xmlNodePtrWrap(obj->user2);
                indexIntoNode = PyInt_FromLong((long) obj->index2);

                tuple = PyTuple_New(2);
                PyTuple_SetItem(tuple, 0, node);
                PyTuple_SetItem(tuple, 1, indexIntoNode);

                PyList_SetItem(list, 1, tuple);

                ret = list;
            }
            break;
        }
        case XPATH_LOCATIONSET:
        {
            xmlLocationSetPtr set;

            set = obj->user;
            if ( set && set->locNr > 0 ) {
                int i;
                PyObject *list;

                list = PyList_New(set->locNr);

                for (i=0; i<set->locNr; i++) {
                    xmlXPathObjectPtr setobj;
                    PyObject *pyobj;

                    setobj = set->locTab[i]; /*xmlXPathObjectPtr setobj*/

                    pyobj = libxml_xmlXPathObjectPtrWrap(setobj);
                    /* xmlXPathFreeObject(setobj) is called */
                    set->locTab[i] = NULL;

                    PyList_SetItem(list, i, pyobj);
                }
                set->locNr = 0;
                ret = list;
            } else {
                Py_INCREF(Py_None);
                ret = Py_None;
            }
            break;
        }
        default:
#ifdef DEBUG
            printf("Unable to convert XPath object type %d\n", obj->type);
#endif
            Py_INCREF(Py_None);
            ret = Py_None;
    }
    xmlXPathFreeObject(obj);
    return (ret);
}

xmlXPathObjectPtr
libxml_xmlXPathObjectPtrConvert(PyObject * obj)
{
    xmlXPathObjectPtr ret = NULL;

#ifdef DEBUG
    printf("libxml_xmlXPathObjectPtrConvert: obj = %p\n", obj);
#endif
    if (obj == NULL) {
        return (NULL);
    }
    if PyFloat_Check
        (obj) {
        ret = xmlXPathNewFloat((double) PyFloat_AS_DOUBLE(obj));

    } else if PyInt_Check(obj) {

        ret = xmlXPathNewFloat((double) PyInt_AS_LONG(obj));

#ifdef PyBool_Check
    } else if PyBool_Check (obj) {

        if (obj == Py_True) {
          ret = xmlXPathNewBoolean(1);
        }
        else {
          ret = xmlXPathNewBoolean(0);
        }
#endif
    } else if PyString_Check
        (obj) {
        xmlChar *str;

        str = xmlStrndup((const xmlChar *) PyString_AS_STRING(obj),
                         PyString_GET_SIZE(obj));
        ret = xmlXPathWrapString(str);
    } else if PyList_Check
        (obj) {
        int i;
        PyObject *node;
        xmlNodePtr cur;
        xmlNodeSetPtr set;

        set = xmlXPathNodeSetCreate(NULL);

        for (i = 0; i < PyList_Size(obj); i++) {
            node = PyList_GetItem(obj, i);
            if ((node == NULL) || (node->ob_type == NULL))
                continue;

            cur = NULL;
            if (PyCObject_Check(node)) {
#ifdef DEBUG
                printf("Got a CObject\n");
#endif
                cur = PyxmlNode_Get(node);
            } else if (PyInstance_Check(node)) {
                PyInstanceObject *inst = (PyInstanceObject *) node;
                PyObject *name = inst->in_class->cl_name;

                if PyString_Check
                    (name) {
                    char *type = PyString_AS_STRING(name);
                    PyObject *wrapper;

                    if (!strcmp(type, "xmlNode")) {
                        wrapper =
                            PyObject_GetAttrString(node, (char *) "_o");
                        if (wrapper != NULL) {
                            cur = PyxmlNode_Get(wrapper);
                        }
                    }
                    }
            } else {
#ifdef DEBUG
                printf("Unknown object in Python return list\n");
#endif
            }
            if (cur != NULL) {
                xmlXPathNodeSetAdd(set, cur);
            }
        }
        ret = xmlXPathWrapNodeSet(set);
    } else {
#ifdef DEBUG
        printf("Unable to convert Python Object to XPath");
#endif
    }
    Py_DECREF(obj);
    return (ret);
}

PyObject *
libxml_xmlValidCtxtPtrWrap(xmlValidCtxtPtr valid)
{
	PyObject *ret;
	
#ifdef DEBUG
	printf("libxml_xmlValidCtxtPtrWrap: valid = %p\n", valid);
#endif
	if (valid == NULL) {
		Py_INCREF(Py_None);
		return (Py_None);
	}

	ret = 
		PyCObject_FromVoidPtrAndDesc((void *) valid,
									 (char *) "xmlValidCtxtPtr", NULL);

	return (ret);
}

PyObject *
libxml_xmlCatalogPtrWrap(xmlCatalogPtr catal)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlNodePtrWrap: catal = %p\n", catal);
#endif
    if (catal == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) catal,
                                     (char *) "xmlCatalogPtr", NULL);
    return (ret);
}

PyObject *
libxml_xmlOutputBufferPtrWrap(xmlOutputBufferPtr buffer)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlOutputBufferPtrWrap: buffer = %p\n", buffer);
#endif
    if (buffer == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) buffer,
                                     (char *) "xmlOutputBufferPtr", NULL);
    return (ret);
}

PyObject *
libxml_xmlParserInputBufferPtrWrap(xmlParserInputBufferPtr buffer)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlParserInputBufferPtrWrap: buffer = %p\n", buffer);
#endif
    if (buffer == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) buffer,
                                     (char *) "xmlParserInputBufferPtr", NULL);
    return (ret);
}

#ifdef LIBXML_REGEXP_ENABLED
PyObject *
libxml_xmlRegexpPtrWrap(xmlRegexpPtr regexp)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlRegexpPtrWrap: regexp = %p\n", regexp);
#endif
    if (regexp == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) regexp,
                                     (char *) "xmlRegexpPtr", NULL);
    return (ret);
}
#endif /* LIBXML_REGEXP_ENABLED */

PyObject *
libxml_xmlTextReaderPtrWrap(xmlTextReaderPtr reader)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlTextReaderPtrWrap: reader = %p\n", reader);
#endif
    if (reader == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) reader,
                                     (char *) "xmlTextReaderPtr", NULL);
    return (ret);
}

PyObject *
libxml_xmlTextReaderLocatorPtrWrap(xmlTextReaderLocatorPtr locator)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlTextReaderLocatorPtrWrap: locator = %p\n", locator);
#endif
    if (locator == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) locator,
                                     (char *) "xmlTextReaderLocatorPtr", NULL);
    return (ret);
}

#ifdef LIBXML_SCHEMAS_ENABLED
PyObject *
libxml_xmlRelaxNGPtrWrap(xmlRelaxNGPtr ctxt)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlRelaxNGPtrWrap: ctxt = %p\n", ctxt);
#endif
    if (ctxt == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) ctxt,
                                     (char *) "xmlRelaxNGPtr", NULL);
    return (ret);
}

PyObject *
libxml_xmlRelaxNGParserCtxtPtrWrap(xmlRelaxNGParserCtxtPtr ctxt)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlRelaxNGParserCtxtPtrWrap: ctxt = %p\n", ctxt);
#endif
    if (ctxt == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) ctxt,
                                     (char *) "xmlRelaxNGParserCtxtPtr", NULL);
    return (ret);
}
PyObject *
libxml_xmlRelaxNGValidCtxtPtrWrap(xmlRelaxNGValidCtxtPtr valid)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlRelaxNGValidCtxtPtrWrap: valid = %p\n", valid);
#endif
    if (valid == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) valid,
                                     (char *) "xmlRelaxNGValidCtxtPtr", NULL);
    return (ret);
}

PyObject *
libxml_xmlSchemaPtrWrap(xmlSchemaPtr ctxt)
{
	PyObject *ret;

#ifdef DEBUG
	printf("libxml_xmlSchemaPtrWrap: ctxt = %p\n", ctxt);
#endif
	if (ctxt == NULL) {
		Py_INCREF(Py_None);
		return (Py_None);
	}
	ret =
		PyCObject_FromVoidPtrAndDesc((void *) ctxt,
									 (char *) "xmlSchemaPtr", NULL);
	return (ret);
}

PyObject *
libxml_xmlSchemaParserCtxtPtrWrap(xmlSchemaParserCtxtPtr ctxt)
{
	PyObject *ret;

#ifdef DEBUG
	printf("libxml_xmlSchemaParserCtxtPtrWrap: ctxt = %p\n", ctxt);
#endif
	if (ctxt == NULL) {
		Py_INCREF(Py_None);
		return (Py_None);
	}
	ret = 
		PyCObject_FromVoidPtrAndDesc((void *) ctxt,
									 (char *) "xmlSchemaParserCtxtPtr", NULL);

	return (ret);
}

PyObject *
libxml_xmlSchemaValidCtxtPtrWrap(xmlSchemaValidCtxtPtr valid)
{
	PyObject *ret;
	
#ifdef DEBUG
	printf("libxml_xmlSchemaValidCtxtPtrWrap: valid = %p\n", valid);
#endif
	if (valid == NULL) {
		Py_INCREF(Py_None);
		return (Py_None);
	}

	ret = 
		PyCObject_FromVoidPtrAndDesc((void *) valid,
									 (char *) "xmlSchemaValidCtxtPtr", NULL);

	return (ret);
}
#endif /* LIBXML_SCHEMAS_ENABLED */

PyObject *
libxml_xmlErrorPtrWrap(xmlErrorPtr error)
{
    PyObject *ret;

#ifdef DEBUG
    printf("libxml_xmlErrorPtrWrap: error = %p\n", error);
#endif
    if (error == NULL) {
        Py_INCREF(Py_None);
        return (Py_None);
    }
    ret =
        PyCObject_FromVoidPtrAndDesc((void *) error,
                                     (char *) "xmlErrorPtr", NULL);
    return (ret);
}
