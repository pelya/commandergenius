/*
 * Copyright (C) 2005 Ronald Lamprecht
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ENIGMA_DOMERRORREPORTER_HH
#define ENIGMA_DOMERRORREPORTER_HH

#include <ostream>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOMError.hpp>

namespace enigma
{
    /**
     * Error handler for DOM parser and serializer.  It reports the error
     * messages according to there severity as a log note or an alert panel
     */
    class DOMErrorReporter : public XERCES_CPP_NAMESPACE_QUALIFIER DOMErrorHandler
    {
    public:
        /**
         * Main constructor.
         * @param aLogStream  the stream for error reports 
         */
        DOMErrorReporter(std::ostream *aLogStream);
        ~DOMErrorReporter();

        /**
         * Implements the DOM ErrorHandler interface.  The error is reported
         * and the maximum error typ is stored for polling evalutaion.
         * @param domError  the error description 
         * @return  true to continue parse or serialization
         */
        bool handleError(const XERCES_CPP_NAMESPACE_QUALIFIER DOMError& domError);

        /**
         * Returns true if error or warning occured since last reset
         */
        bool getSawErrors() const;

        /**
         * Returns maximum severity of error since last reset
         */
        XERCES_CPP_NAMESPACE_QUALIFIER DOMError::ErrorSeverity getSeverity() const;
        
        /**
         * Resets error flag and severity
         */
        void resetErrors();
        
        /**
         * Switch on report output to log stream
         */
        void reportToLog();
        
        /**
         * Switch on report output to err stream
         */
        void reportToErr();
        
        /**
         * Switch off report output
         */
        void reportToNull();
        
        /**
         * Switch on report output to given stream
         */
        void reportToOstream(std::ostream *anOstream);
    
    private :
        //  Unimplemented constructors and operators
        DOMErrorReporter(const DOMErrorReporter&);
        void operator=(const DOMErrorReporter&);
    
        /**
         * Flag for warnings or errors that occured since last reset
         */
        bool    sawErrors;

        /**
         * Most significant error type since last reset
         */
        XERCES_CPP_NAMESPACE_QUALIFIER DOMError::ErrorSeverity severity;
        
        /**
         * Current output stream for error reports
         */
        std::ostream *reportStream;
        
        /**
         * The configured log output stream
         */
        std::ostream *logStream;
    };
    
} // namespace enigma
#endif

