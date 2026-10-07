/*
 * Copyright 2021 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * This is a server utility program for exposing realtime
 * monitoring items to prometheus.
 */

# define NEED_SLEEP
# define NEED_SIGNAL

# include <stdhdrs.h>

// NT header defines conflict with P4API methods
# ifdef GetMessage
# undef GetMessage
# endif

# include <strbuf.h>
# include <strops.h>
# include <error.h>
# include <errorlog.h>
# include <options.h>
# include <debug.h>

# include <ident.h>

# include <memfile.h>
# include <monitem.h>

# include <prometheus/counter.h>
# include <prometheus/exposer.h>
# include <prometheus/registry.h>

// Version string helper
Ident ident = { 
	IdentMagic "P4Mon Prometheus Exporter" "/"
	ID_OS "/" ID_REL "/" ID_PATCH,
	ID_Y "/" ID_M "/" ID_D 
};

static const char long_usage[] =
"Usage:\n"
"\n"
"    p4mon-prometheus-exporter [ -h [ -l ] ] [ -V ] [ -L port ] [ monfile.mem ]\n"
"\n"
"    Options:\n"
"	-h		print this message\n"
"	-h -l		print this message and legal information\n"
"	-V		print version\n"
"	-L port		port on which to start the HTTP server\n"
"\n"
"    The single argument provided to p4mon-prometheus-exporter is\n"
"    the path to P4D's rt.monitorfile shared memory file (defaults\n"
"    to monfile.mem in the current working directory).\n"
"\n";

// Wrap the messages in a class so they're easy to define after the code
// Legal is long.
class MsgMon {
    public:
	static ErrorId Usage;
	static ErrorId Legal;
};

class MemItemsCollectable : public prometheus::Collectable, MemItems
{
    public:
	MemItemsCollectable() : MemItems() {}
	
	void
	Load( const char *p, Error *e )
	{
	    MemItems::Load( p, e );
	}

	std::vector<prometheus::MetricFamily>
	Collect() const override
	{
	    std::vector<prometheus::MetricFamily> vec
	        = std::vector<prometheus::MetricFamily>();
	    for(MonItem *i = First(); i; i = Next(i))
	    {
	        if( i->ItemId() == MIT_NONE || i->Flags() & MI_HIDDEN )
	            continue;

	        // Prometheus doesn't like .'s in names
	        StrBuf name, tmp;
	        StrOps::Replace( tmp, i->Name(),
	                         StrRef( "." ), StrRef( "_" ) );

	        // Make it clear this is a Perforce Server metric
	        name = "p4d_";
	        name << tmp;

	        switch( i->ItemId() )
	        {
	        case MIT_INTMAX:
	            {
	                prometheus::MetricFamily mf;
	                prometheus::ClientMetric cm;
	                cm.counter.value = static_cast<double>(
	                                     ((MonIntMax *)i)->MaxValue());
	                StrBuf mName = name.Text();
	                mName << "_max";
	                mf.name = mName.Text();
	                mf.type = prometheus::MetricType::Counter;
	                mf.metric.push_back( cm );
	                vec.push_back( mf );
	            }
	        case MIT_INT:
	            {
	                prometheus::MetricFamily mf;
	                prometheus::ClientMetric cm;
	                mf.name = name.Text();
	                if( i->Flags() & MI_CUMULATIVE )
	                {
	                    cm.counter.value = static_cast<double>(
	                                     ((MonInteger *)i)->Value());
	                    mf.type = prometheus::MetricType::Counter;
	                }
	                else
	                {
	                    cm.gauge.value = static_cast<double>(
	                                     ((MonInteger *)i)->Value());
	                    mf.type = prometheus::MetricType::Gauge;
	                }
	                mf.metric.push_back( cm );
	                vec.push_back( mf );
	                break;
	            }
	        }
	    }
	    return vec;
	}
};

int
main(int cargc, char **cargv)
{
	AssertLog.SetTag( "Perforce Realtime Monitoring Prometheus Exporter" );
	AssertLog.EnableCritSec();
	AssertError.Set( E_INFO, "P4Mon Prometheus Exporter" );
	AssertError.Clear();

	Error e;

	int argc = cargc;
	StrRef *oargv = new StrRef[ argc ];
	StrPtr *argv = oargv;

	for( int ac = 0; ac < argc; ac++ )
	    argv[ac] = StrRef( cargv[ac] );

	argc--, argv++;
	Options opts;
	opts.Parse( argc, argv, "L:hlV", OPT_OPT, MsgMon::Usage, &e );

	AssertLog.Abort( &e );

	if( opts[ 'h' ] )
	{
	    printf( long_usage );

	    if( opts['l'] )
	    {
	        e.Set( MsgMon::Legal );
	        StrBuf tmp;
	        e.Fmt( &tmp );
	        printf( "\n\n%s", tmp.Text() );
	    }

	    return 0;
	}
	
	if( opts['l'] )
	{
	    e.Set( MsgMon::Usage );
	    AssertLog.Abort( &e );
	}
	
	if( opts[ 'V' ] )
	{
	    StrBuf s;
	    ident.GetMessage( &s );
	    printf( "%s", s.Text() );
	    return 0;
	}
	
	// create a http server running on port 8080
	StrBuf port = "0.0.0.0:8080";
	if( opts['L'] )
	    port = *opts['L'];

	std::shared_ptr<MemItemsCollectable> mi = std::make_shared<MemItemsCollectable>();
	
	mi->Load( argc ? argv->Text() : "monfile.mem", &e );
	if( e.Test() )
	    AssertLog.Abort( &e );

	try
	{
	    prometheus::Exposer exposer( port.Text() );
	    exposer.RegisterCollectable( mi );

	    while( true )
	        sleep( 1000 );
	}
	catch (std::exception& ex)
	{
	    e.Set( E_FAILED, ex.what() );
	    e.Set( E_FAILED, "Exception thrown by Prometheus Exposer:" );
	    AssertLog.Abort( &e );
	}
	
	return 0;
}


ErrorId MsgMon::Usage = { ErrorOf( 0, 0, E_FATAL, 0, 0 ),
	"p4mon-prometheus-exporter [ -h [ -l ] ] [ -V ] [ -L port ] [ monfile.mem ]" };

ErrorId MsgMon::Legal = { ErrorOf( 0, 0, E_INFO, 0, 0 ),
"\n"
"    The following are the license statements for code used in\n"
"    this program.\n"
"\n"
"    See 'p4 help legal' for additional information.\n"
"\n"
"\n"
"    prometheus-cpp License:\n"
"    -----------------------\n"
"\n"
"    MIT License\n"
"\n"
"    Copyright (c) 2016-2019 Jupp Mueller\n"
"    Copyright (c) 2017-2019 Gregor Jasny\n"
"\n"
"    And many contributors, see\n"
"    https://github.com/jupp0r/prometheus-cpp/graphs/contributors\n"
"\n"
"    Permission is hereby granted, free of charge, to any person obtaining a\n"
"    copy of this software and associated documentation files (the \"Software\"),\n"
"    to deal in the Software without restriction, including without limitation\n"
"    the rights to use, copy, modify, merge, publish, distribute, sublicense,\n"
"    and/or sell copies of the Software, and to permit persons to whom the\n"
"    Software is furnished to do so, subject to the following conditions:\n"
"\n"
"    The above copyright notice and this permission notice shall be included in\n"
"    all copies or substantial portions of the Software.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n"
"    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
"    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
"    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
"    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"
"    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n"
"    DEALINGS IN THE SOFTWARE.\n"
"\n"
"\n"
"    Civetweb License:\n"
"    -----------------------\n"
"\n"
"    Copyright (c) 2013-2021 The CivetWeb developers (CREDITS.md)\n"
"\n"
"    Copyright (c) 2004-2013 Sergey Lyubka\n"
"\n"
"    Copyright (c) 2013 No Face Press, LLC (Thomas Davis)\n"
"\n"
"    Copyright (c) 2013 F-Secure Corporation\n"
"\n"
"    Permission is hereby granted, free of charge, to any person obtaining a\n"
"    copy of this software and associated documentation files (the \"Software\"),\n"
"    to deal in the Software without restriction, including without limitation\n"
"    the rights to use, copy, modify, merge, publish, distribute, sublicense,\n"
"    and/or sell copies of the Software, and to permit persons to whom the\n"
"    Software is furnished to do so, subject to the following conditions:\n"
"\n"
"    The above copyright notice and this permission notice shall be included in\n"
"    all copies or substantial portions of the Software.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
"    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
"    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
"    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
"    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"
"    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n"
"    DEALINGS IN THE SOFTWARE.\n"
"\n"
"\n"
"    P4/P4API License\n"
"    -----------------------\n"
"    Copyright (c) 1995-" ID_Y ", Perforce Software, Inc.\n"
"    All rights reserved.\n"
"\n"
"    Redistribution and use in source and binary forms, with or without\n"
"    modification, are permitted provided that the following conditions are met:\n"
"\n"
"        Redistributions of source code must retain the above copyright notice,\n"
"        this list of conditions and the following disclaimer.\n"
"\n"
"        Redistributions in binary form must reproduce the above copyright\n"
"        notice, this list of conditions and the following disclaimer in the\n"
"        documentation and/or other materials provided with the distribution.\n"
"\n"
"    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"\n"
"    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
"    IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"
"    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE\n"
"    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR\n"
"    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF\n"
"    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS\n"
"    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN\n"
"    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)\n"
"    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE\n"
"    POSSIBILITY OF SUCH DAMAGE.\n"
"\n"
"\n"
"    OpenSSL:\n"
"    -----------------------\n"
"	This product includes cryptographic software written by Eric Young\n"
"	(eay@cryptsoft.com).\n"
"	This product includes software written by Tim Hudson\n"
"	(tjh@cryptsoft.com).\n"
"\n"
"    OpenSSL License\n"
"    -----------------------\n"
"\n"
"     * ====================================================================\n"
"     * Copyright (c) 1998-2011 The OpenSSL Project.  All rights reserved.\n"
"     *\n"
"     * Redistribution and use in source and binary forms, with or without\n"
"     * modification, are permitted provided that the following conditions\n"
"     * are met:\n"
"     *\n"
"     * 1. Redistributions of source code must retain the above copyright\n"
"     *    notice, this list of conditions and the following disclaimer.\n"
"     *\n"
"     * 2. Redistributions in binary form must reproduce the above copyright\n"
"     *    notice, this list of conditions and the following disclaimer in\n"
"     *    the documentation and/or other materials provided with the\n"
"     *    distribution.\n"
"     *\n"
"     * 3. All advertising materials mentioning features or use of this\n"
"     *    software must display the following acknowledgment:\n"
"     *    \"This product includes software developed by the OpenSSL Project\n"
"     *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)\n"
"     *\n"
"     * 4. The names \"OpenSSL Toolkit\" and \"OpenSSL Project\" must not be\n"
"     *    used to endorse or promote products derived from this software\n"
"     *    without prior written permission. For written permission, please\n"
"     *    contact openssl-core@openssl.org.\n"
"     *\n"
"     * 5. Products derived from this software may not be called \"OpenSSL\n"
"     *    nor may \"OpenSSL\" appear in their names without prior written\n"
"     *    permission of the OpenSSL Project.\n"
"     *\n"
"     * 6. Redistributions of any form whatsoever must retain the following\n"
"     *    acknowledgment:\n"
"     *    \"This product includes software developed by the OpenSSL Project\n"
"     *    for use in the OpenSSL Toolkit (http://www.openssl.org/)\n"
"     *\n"
"     * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY\n"
"     * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
"     * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n"
"     * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR\n"
"     * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n"
"     * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT\n"
"     * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n"
"     * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n"
"     * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,\n"
"     * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)\n"
"     * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED\n"
"     * OF THE POSSIBILITY OF SUCH DAMAGE.\n"
"     * ====================================================================\n"
"     *\n"
"     * This product includes cryptographic software written by Eric Young\n"
"     * (eay@cryptsoft.com).  This product includes software written by Tim\n"
"     * Hudson (tjh@cryptsoft.com).\n"
"     *\n"
"     *\n"
"\n"
"    Original SSLeay License\n"
"    -----------------------\n"
"\n"
"     * Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)\n"
"     * All rights reserved.\n"
"     *\n"
"     * This package is an SSL implementation written\n"
"     * by Eric Young (eay@cryptsoft.com).\n"
"     * The implementation was written so as to conform with Netscapes SSL.\n"
"     *\n"
"     * This library is free for commercial and non-commercial use as long as\n"
"     * the following conditions are adhered to.  The following conditions\n"
"     * apply to all code found in this distribution, be it the RC4, RSA,\n"
"     * lhash, DES, etc., code; not just the SSL code.  The SSL documentation\n"
"     * included with this distribution is covered by the same copyright terms\n"
"     * except that the holder is Tim Hudson (tjh@cryptsoft.com).\n"
"     *\n"
"     * Copyright remains Eric Young's, and as such any Copyright notices in\n"
"     * the code are not to be removed.\n"
"     * If this package is used in a product, Eric Young should be given\n"
"     * attribution as the author of the parts of the library used.\n"
"     * This can be in the form of a textual message at program startup or\n"
"     * in documentation (online or textual) provided with the package.\n"
"     *\n"
"     * Redistribution and use in source and binary forms, with or without\n"
"     * modification, are permitted provided that the following conditions\n"
"     * are met:\n"
"     * 1. Redistributions of source code must retain the copyright\n"
"     *    notice, this list of conditions and the following disclaimer.\n"
"     * 2. Redistributions in binary form must reproduce the above copyright\n"
"     *    notice, this list of conditions and the following disclaimer in the\n"
"     *    documentation and/or other materials provided with the distribution.\n"
"     * 3. All advertising materials mentioning features or use of this\n"
"     *    software must display the following acknowledgement:\n"
"     *    \"This product includes cryptographic software written by\n"
"     *     Eric Young (eay@cryptsoft.com)\"\n"
"     *    The word 'cryptographic' can be left out if the routines from the\n"
"     *    library being used are not cryptographic related  :-) .\n"
"     * 4. If you include any Windows specific code (or a derivative thereof)\n"
"     *    from the apps directory (application code) you must include an\n"
"     *    acknowledgement: \"This product includes software written by Tim\n"
"     *    Hudson (tjh@cryptsoft.com)\"\n"
"     *\n"
"     * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND\n"
"     * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
"     * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n"
"     * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS\n"
"     * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR\n"
"     * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF\n"
"     * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR\n"
"     * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\n"
"     * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE\n"
"     * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN\n"
"     * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
"     *\n"
"     * The licence and distribution terms for any publicly available version\n"
"     * or derivative of this code cannot be changed.  i.e. this code cannot\n"
"     * simply be copied and put under another distribution licence\n"
"     * [including the GNU Public Licence.]\n"
"     *\n"
"\n"
"\n"
"    sol2 License\n"
"    -----------------------\n"
"\n"
"    Copyright (c) 2013-2018 Rapptz, ThePhD, and contributors\n"
"\n"
"    Permission is hereby granted, free of charge, to any person obtaining\n"
"    a copy of this software and associated documentation files (the\n"
"    \"Software\"), to deal in the Software without restriction, including\n"
"    without limitation the rights to use, copy, modify, merge, publish,\n"
"    distribute, sublicense, and/or sell copies of the Software, and to\n"
"    permit persons to whom the Software is furnished to do so, subject to\n"
"    the following conditions:\n"
"\n"
"    The above copyright notice and this permission notice shall be\n"
"    included in all copies or substantial portions of the Software.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
"    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
"    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n"
"    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE\n"
"    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION\n"
"    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION\n"
"    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
"\n"
"\n"
"    Lua License\n"
"    -----------------------\n"
"\n"
"    Copyright (c) 1994-2018 Lua.org, PUC-Rio.\n"
"\n"
"    Permission is hereby granted, free of charge, to any person obtaining\n"
"    a copy of this software and associated documentation files (the\n"
"    \"Software\"), to deal in the Software without restriction, including\n"
"    without limitation the rights to use, copy, modify, merge, publish,\n"
"    distribute, sublicense, and/or sell copies of the Software, and to\n"
"    permit persons to whom the Software is furnished to do so, subject to\n"
"    the following conditions:\n"
"\n"
"    The above copyright notice and this permission notice shall be\n"
"    included in all copies or substantial portions of the Software.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
"    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
"    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n"
"    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE\n"
"    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION\n"
"    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION\n"
"    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
"\n"
"\n"
"    cURL License\n"
"    -----------------------\n"
"\n"
"    This code uses a permissive license. Please use the following\n"
"    attribution in the documentation of the open source code.\n"
"\n"
"    Copyright (c) 1996 - 2018, Daniel Stenberg, daniel@haxx.se, and many\n"
"    contributors, see the THANKS file.\n"
"\n"
"    All rights reserved.\n"
"\n"
"    Permission to use, copy, modify, and distribute this software for any\n"
"    purpose with or without fee is hereby granted, provided that the above\n"
"    copyright notice and this permission notice appear in all copies.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n"
"    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
"    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF\n"
"    THIRD PARTY RIGHTS. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS\n"
"    BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN\n"
"    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN\n"
"    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
"    SOFTWARE.\n"
"\n"
"    Except as contained in this notice, the name of a copyright holder shall\n"
"    not be used in advertising or otherwise to promote the sale, use or\n"
"    other dealings in this Software without prior written authorization of\n"
"    the copyright holder.\n"
"\n"
"\n"
"    Lua-cURLv3 License\n"
"    -----------------------\n"
"\n"
"    Copyright (c) 2014-2018 Alexey Melnichuk\n"
"\n"
"    Permission is hereby granted, free of charge, to any person obtaining a\n"
"    copy of this software and associated documentation files (the\n"
"    \"Software\"), to deal in the Software without restriction, including\n"
"    without limitation the rights to use, copy, modify, merge, publish,\n"
"    distribute, sublicense, and/or sell copies of the Software, and to permit\n"
"    persons to whom the Software is furnished to do so, subject to the\n"
"    following conditions:\n"
"\n"
"    The above copyright notice and this permission notice shall be included\n"
"    in all copies or substantial portions of the Software.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n"
"    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
"    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN\n"
"    NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,\n"
"    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR\n"
"    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE\n"
"    USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
"\n"
"\n"
"    LuaSQLite3 License\n"
"    -----------------------\n"
"\n"
"    Copyright (C) 2002-2016 Tiago Dionizio, Doug Currie\n"
"    All rights reserved.\n"
"    Author : Tiago Dionizio <tiago.dionizio@ist.utl.pt>\n"
"    Author : Doug Currie <doug.currie@alum.mit.edu>\n"
"    Library : lsqlite3 - an SQLite 3 database binding for Lua 5\n"
"\n"
"    Permission is hereby granted, free of charge, to any person obtaining\n"
"    a copy of this software and associated documentation files (the\n"
"    \"Software\"), to deal in the Software without restriction, including\n"
"    without limitation the rights to use, copy, modify, merge, publish,\n"
"    distribute, sublicense, and/or sell copies of the Software, and to\n"
"    permit persons to whom the Software is furnished to do so, subject to\n"
"    the following conditions:\n"
"\n"
"    The above copyright notice and this permission notice shall be\n"
"    included in all copies or substantial portions of the Software.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
"    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
"    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n"
"    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\n"
"    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,\n"
"    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE\n"
"    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
"\n"
"\n"
"    lua-cjson License\n"
"    -----------------------\n"
"\n"
"    Copyright (c) 2010-2012 Mark Pulford <mark@kyne.com.au>\n"
"\n"
"    Permission is hereby granted, free of charge, to any person obtaining\n"
"    a copy of this software and associated documentation files (the\n"
"    \"Software\"), to deal in the Software without restriction, including\n"
"    without limitation the rights to use, copy, modify, merge, publish,\n"
"    distribute, sublicense, and/or sell copies of the Software, and to\n"
"    permit persons to whom the Software is furnished to do so, subject to\n"
"    the following conditions:\n"
"\n"
"    The above copyright notice and this permission notice shall be\n"
"    included in all copies or substantial portions of the Software.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
"    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
"    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n"
"    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\n"
"    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,\n"
"    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE\n"
"    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
"\n"
"\n"
"    libc++ and libc++abi License\n"
"    -----------------------\n"
"\n"
"    These libraries are dual-licensed:\n"
"\n"
"    The University of Illinois/NCSA Open Source License (NCSA)\n"
"    Developed under the LLVM Project\n"
"\n"
"    Permission is hereby granted, free of charge, to any person obtaining a\n"
"    copy of this software and associated documentation files (the\n"
"    \"Software\"), to deal with the Software without restriction, including\n"
"    without limitation the rights to use, copy, modify, merge, publish,\n"
"    distribute, sublicense, and/or sell copies of the Software, and to\n"
"    permit persons to whom the Software is furnished to do so, subject to the\n"
"    following conditions:\n"
"\n"
"    Redistributions of source code must retain the above copyright notice,\n"
"    this list of conditions and the following disclaimers.\n"
"    Redistributions in binary form must reproduce the above copyright\n"
"    notice, this list of conditions and the following disclaimers in the\n"
"    documentation and/or other materials provided with the distribution.\n"
"    Neither the names of <Name of Development Group, Name of Institution>,\n"
"    nor the names of its contributors may be used to endorse or promote\n"
"    products derived from this Software without specific prior written\n"
"    permission.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n"
"    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
"    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n"
"    IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR\n"
"    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,\n"
"    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE\n"
"     SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.\n"
"\n"
"    MIT\n"
"    Developed under the LLVM Project\n"
"\n"
"    Permission is hereby granted, free of charge, to any person obtaining a\n"
"    copy of this software and associated documentation files (the\n"
"    \"Software\"), to deal in the Software without restriction, including\n"
"    without limitation the rights to use, copy, modify, merge, publish,\n"
"    distribute, sublicense, and/or sell copies of the Software, and to permit\n"
"    persons to whom the Software is furnished to do so, subject to the\n"
"    following conditions:\n"
"\n"
"    The above copyright notice and this permission notice shall be included\n"
"    in all copies or substantial portions of the Software.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n"
"    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
"    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN\n"
"    NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,\n"
"    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR\n"
"    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE\n"
"    USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
"\n"
"\n"
"    jemalloc\n"
"    -----------------------\n"
"\n"
"    Copyright (C) 2002-present Jason Evans <jasone@canonware.com>.\n"
"    All rights reserved.\n"
"    Copyright (C) 2007-2012 Mozilla Foundation.  All rights reserved.\n"
"    Copyright (C) 2009-present Facebook, Inc.  All rights reserved.\n"
"\n"
"    Redistribution and use in source and binary forms, with or without\n"
"    modification, are permitted provided that the following conditions are met:\n"
"    1. Redistributions of source code must retain the above copyright\n"
"       notice(s), this list of conditions and the following disclaimer.\n"
"    2. Redistributions in binary form must reproduce the above copyright\n"
"       notice(s), this list of conditions and the following disclaimer in the\n"
"       documentation and/or other materials provided with the distribution.\n"
"\n"
"    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY\n"
"    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
"    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
"    DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY\n"
"    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n"
"    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR\n"
"    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER\n"
"    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n"
"    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n"
"    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n"
"    SUCH DAMAGE.\n"
"\n"
"\n"
"    JSON for Modern C++\n"
"    -----------------------\n"
"\n"
"    Copyright (c) 2013-2018 Niels Lohmann\n"
"\n"
"    Permission is hereby granted, free of charge, to any person obtaining\n"
"    a copy of this software and associated documentation files (the\n"
"    \"Software\"), to deal in the Software without restriction, including\n"
"    without limitation the rights to use, copy, modify, merge, publish,\n"
"    distribute, sublicense, and/or sell copies of the Software, and to permit\n"
"    persons to whom the Software is furnished to do so, subject to the\n"
"    following conditions:\n"
"\n"
"    The above copyright notice and this permission notice shall be included\n"
"    in all copies or substantial portions of the Software.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
"    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
"    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n"
"    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\n"
"    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT\n"
"    OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR\n"
"    THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
"\n"

"    optional-lite and any-lite\n"
"    Martin Moene\n"
"    -----------------------\n"
"\n"
"    Boost Software License - Version 1.0 - August 17th, 2003\n"
"\n"
"    Permission is hereby granted, free of charge, to any person or organization\n"
"    obtaining a copy of the software and accompanying documentation covered by\n"
"    this license (the \"Software\") to use, reproduce, display, distribute,\n"
"    execute, and transmit the Software, and to prepare derivative works of the\n"
"    Software, and to permit third-parties to whom the Software is furnished to\n"
"    do so, all subject to the following:\n"
"\n"
"    The copyright notices in the Software and this entire statement, including\n"
"    the above license grant, this restriction and the following disclaimer,\n"
"    must be included in all copies of the Software, in whole or in part, and\n"
"    all derivative works of the Software, unless such copies or derivative\n"
"    works are solely in the form of machine-executable object code generated by\n"
"    a source language processor.\n"
"\n"
"    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
"    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
"    FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT\n"
"    SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE\n"
"    FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,\n"
"    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n"
"    DEALINGS IN THE SOFTWARE.\n"
"\n"
};
