/*
 * Copyright (C) 2010, Stefan Klanke
 * Donders Institute for Donders Institute for Brain, Cognition and Behaviour,
 * Centre for Cognitive Neuroimaging, Radboud University Nijmegen,
 * Kapittelweg 29, 6525 EN Nijmegen, The Netherlands
 */

#ifndef __FtBuffer_h
#define __FtBuffer_h

#include <buffer.h>
#include <SimpleStorage.h>

/** Simple wrapper class for FieldTrip buffer request. Not complete yet.
*/

class FtBufferRequest {
	public:
	
	FtBufferRequest() : m_buf() {
		m_def.version = VERSION;
		m_msg.def = &m_def;
	}
	
	void prepGetHeader() {
		m_def.command = GET_HDR;
		m_def.bufsize = 0;
		m_msg.buf = NULL;		
	}

	bool prepPutData(UINT32_T numChannels, UINT32_T numSamples, UINT32_T dataType, const void *data) {
		// This is for safety: If a user ignores this function returning false,
		// and just sends this request to the buffer server, we need to make sure
		// no harm is done - so we sent a small invalid packet
		m_def.command = GET_ERR; 
		m_def.bufsize = 0;
		
		UINT32_T  wordSize = wordsize_from_type(dataType);
		if (wordSize == 0) return false;
		
		UINT32_T  dataSize = wordSize * numSamples * numChannels;
		UINT32_T  totalSize = dataSize + sizeof(datadef_t);
		datadef_t *dd;
		
		if (!m_buf.resize(totalSize)) return false;
		dd = (datadef_t *) m_buf.data();
		dd->nchans = numChannels;
		dd->nsamples = numSamples;
		dd->data_type = dataType;
		dd->bufsize = dataSize;
		// dd+1 points to the next byte after the data_def
		memcpy(dd+1, data, dataSize);
		m_def.command = PUT_DAT;
		m_def.bufsize = totalSize;
		return true;
	}
	
	void prepGetData(UINT32_T begsample, UINT32_T endsample) {
		m_def.command = GET_DAT;
		m_msg.buf = &m_extras.ds;
		m_def.bufsize = sizeof(datasel_t);
		m_extras.ds.begsample = begsample;
		m_extras.ds.endsample = endsample;
	}
		
	void prepWaitData(UINT32_T threshold, UINT32_T milliseconds) {
		m_def.command = WAIT_DAT;
		m_msg.buf = &m_extras.wd;
		m_def.bufsize = sizeof(waitdef_t);
		m_extras.wd.threshold = threshold;
		m_extras.wd.milliseconds = milliseconds;
	}

	const message_t *out() const {
		return &m_msg;
	}
		
	protected:
	
	SimpleStorage m_buf;
	message_t m_msg;
	messagedef_t m_def;
	// we'll always use only one of these at a time
	// so let's define a union to save some (stack) space
	union {	
		waitdef_t wd;
		datasel_t ds;
	} m_extras;
};


/** Simple wrapper class for FieldTrip buffer responses. Not complete yet.
*/
class FtBufferResponse {
	public:
	
	FtBufferResponse() {
		m_response = NULL;
	}
		
	bool checkGetHeader(headerdef_t &hdr, SimpleStorage *bufStore = NULL) const {
		if (m_response == NULL) return false;
		if (m_response->def == NULL) return false;
		if (m_response->def->version != VERSION) return false;
		if (m_response->def->command != GET_OK) return false;
		if (m_response->def->bufsize < sizeof(headerdef_t)) return false;
		if (m_response->buf == NULL) return false;
		
		memcpy(&hdr, m_response->buf, sizeof(headerdef_t));
		if (bufStore != NULL) {
			unsigned int len = m_response->def->bufsize - sizeof(headerdef_t);
			char *src = (char *) m_response->buf + sizeof(headerdef_t);
			if (!bufStore->resize(len)) return false;
			memcpy(bufStore->data(), src, len);
		}		
		return true;
	}
	
	bool checkGetData(datadef_t &datadef, SimpleStorage *datStore = NULL) const {
		if (m_response == NULL) return false;
		if (m_response->def == NULL) return false;
		if (m_response->def->version != VERSION) return false;
		if (m_response->def->command != GET_OK) return false;
		if (m_response->def->bufsize < sizeof(datadef_t)) return false;
		if (m_response->buf == NULL) return false;
		
		memcpy(&datadef, m_response->buf, sizeof(datadef_t));
		if (datStore != NULL) {
			unsigned int len = m_response->def->bufsize - sizeof(datadef_t);
			char *src = (char *) m_response->buf + sizeof(datadef_t);
			if (!datStore->resize(len)) return false;
			memcpy(datStore->data(), src, len);
		}		
		return true;
	}
	
	bool checkWait(unsigned int &nSamples) const {
		if (m_response == NULL) return false;
		if (m_response->def == NULL) return false;
		if (m_response->def->version != VERSION) return false;
		if (m_response->def->command != WAIT_OK) return false;
		if (m_response->def->bufsize != sizeof(UINT32_T)) return false;
		if (m_response->buf == NULL) return false;
		nSamples = *((UINT32_T *)m_response->buf);
		return true;
	}	
	
	message_t **in() {
		if (m_response != NULL) clearResponse();
		return &m_response;
	}
	
	bool checkPut() {
		if (m_response == NULL) return false;
		if (m_response->def == NULL) return false;
		if (m_response->def->version != VERSION) return false;
		return m_response->def->command == PUT_OK;
	}
	
	bool checkFlush() {
		if (m_response == NULL) return false;
		if (m_response->def == NULL) return false;
		if (m_response->def->version != VERSION) return false;
		return m_response->def->command == FLUSH_OK;
	}	
	
	~FtBufferResponse() {
		if (m_response != NULL) clearResponse();
	}

	void clearResponse() {
		if (m_response->buf != NULL) free(m_response->buf);
		if (m_response->def != NULL) free(m_response->def);
		free(m_response);
		m_response = NULL;
	}
	
	message_t *m_response;
};

#endif
