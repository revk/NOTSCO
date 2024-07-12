#!../login/envcgi /bin/csh -f
source ../script/loggedin
  
echo Content-Type: text/html
echo ""
  
xmlsql -d notsco head.html - tail.html << 'END'
<h1>Tips, and notes on TOTSCO specifications</h1>
<p>Various lessons learned which may be useful to you.</p>
<h2>URL</h2>
<p>API§2.1.2 states <i>The API URI format provided by the TOTSCo Hub, and each RCP (or MAP) will conform to the following convention <tt>https://{fqdn}/letterbox/{version}/post</tt></i>. But it seems TOTSCO do not follow this themselves in that their simulator does not use <tt>letterbox</tt>, and also, it seems they will let you have any URI for token and post. That is nice, but would be nice if the specification made that clear. This will make it easier for CPs that may find other paths easier to implement on their servers.</p>
<h2>RCPID</h2>
<p>OTS§2.2.1 states <i>RCPIDs are 4 alpha chars... for clarity RCPIDs will not start with A</i>. This seems pretty clear, and so obviously we included RCPID in all databases as a <tt>char(4)</tt>, only to find the list of RCPs includes one with an RCPID of <tt>TOTSCO</tt> (who are clearly not an RCP even!). This means when doing a list of CPs to customers you need to exclude TOTSCO (and yourself). TOTSCO have said (TOTSCO-33820) that OTS§2.2.1 is simply a note by the writer to differenciate RCPID from NetworkOperator, and that RCPDID is not specified in the specification. The seem to have no problem with it not being specified!!!</p>
<h2>Response times</h2>
<p>The message delivery policies say you have to respond at an HTTP level within 3 seconds, with a 1 second initial connection timeout. Oddly, in email, TOTSCO said it is 2 seconds. This is quite tight, and it seems keep-alives are requested by TOTSCO, and in some circumstances apache will actually wait 5 seconds even after a CGI has finished. We fixed this by using <tt>Content-Length:</tt> on replies, but you may be able to disable KeepAlive in your apache config. This is extra tricky as it is not obvious there has been a failure, the TCP closes cleanly, but TOTSCO keep trying to send the message over and over again. This NOTSCO simulator sets keep-alive as well, but we are not sure it triggers the exact issue, so watch out for this.</p>
<h2>correlationID</h2>
<p>When developing our system, we initially (sensibly) assumed the lifetime of a  <tt>correlationID</tt> was from a <tt>Request</tt> message to a <tt>Confirmation</tt> or <tt>Failure</tt> message. This is simple, and allows a reply to be tied to a request, especially a <tt>matchRequest</tt>. However, the TOTSCO test cases clearly showed messages like <tt>residentialSwitchOrderRequest</tt> quoting a destination correlationID, which, as per API§2.1.5 <i>would only be populated when the message is being sent in response to a message previously sent to you</i>, clearly suggesting the <tt>residentialSwitchOrderRequest</tt> is seen as a <i>reply</i> to the <tt>residentialSwitchMatchConfirmation</tt>. So we spent some time coding a system to track correlationID over the whole switch order.</p>
<p>TOTSCO have now confirmed (TOTSCO-34021) <i>We would like to inform you that, according to the specification, a switch order request is not seen as a response to a match confirmation. Additionally, the TOTSCo hub does not require users to include a destination correlation ID in any request message.</i> This means the test cases are just just wrong, or at least misleading.</p>
<p>Note that API§2.1.5 also states <i>In a source element, the correlationID must always be provided</i>, but this is contradicted by API§2.1.8 which explains <tt>messageDeliveryFailure</tt> does not have a source correlationID.</p>
<h2>Network Operator</h2>
<p>OTS§2.2.1 says of NetworkOperator: <i>Network operators will be identified by A and 3 numbers, e.g. A001. “A” should be memorable as “Access Provider”</i> which is nice and clear.</p>
<p>It then goes on to say <i>If the NBICS is found as VoIP service, the network operator can be retuned as “VOIP”</i> which directly contradicts that. It makes no sense as any number on the PSTN, even if it uses VoIP to connect to a customer, will have an actual network opertator, which the gaining provider will need to know.</p>
<p>The Access Network Operators List defines <tt>A000</tt> as VOIP, so presumably they actually mean use "A000" not "VOIP" as a NetworkOperator.</p>
<p>But then OTS§3.2.1 has an example with NetworkOperator as "VOIP".</p>
<p>TOTSCO have now confirmed (TOTSCO-34207) that it should be <tt>A000</tt> that is used.</p>
<h2>Empty strings</h2>
<p>It looks like all simple types are done as JSON strings, though I am not sure the spec says that, and even, in one place, says a field is <i>numeric</i> even though the example then has a string with numbers in it.</p>
<p>Also, in at lease one example, a field that is <i>optional</i> and <i>would not be populated</i> is actually included as an empty string. What we have chosen to do is not send optional fields for which we have no value, but to accept an empty string in such a field as if it was not populated. Again, it would be nice if the specification was clear.</p>
<h2>UUID</h2>
<p>A <i>Switch order Reference</i> is specified as a UUID. This is defined by RFC 9562 to allow upper or lower case, so you should match to either.</p>
'END'
