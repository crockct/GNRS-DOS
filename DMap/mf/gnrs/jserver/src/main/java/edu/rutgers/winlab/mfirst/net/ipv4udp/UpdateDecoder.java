package edu.rutgers.winlab.mfirst.net.ipv4udp;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.messages.UpdateMessage;
import edu.rutgers.winlab.mfirst.messages.MessageType;
import edu.rutgers.winlab.mfirst.messages.opt.Option;
import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import org.apache.mina.core.buffer.IoBuffer;
import org.apache.mina.core.session.IoSession;
import org.apache.mina.filter.codec.ProtocolDecoderOutput;
import org.apache.mina.filter.codec.demux.MessageDecoder;
import org.apache.mina.filter.codec.demux.MessageDecoderResult;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;

/**
 * Created by wontoniii on 4/24/15.
 */
public class UpdateDecoder implements MessageDecoder {
    private static final Logger LOG = LoggerFactory
            .getLogger(UpdateDecoder.class);

    @Override
    public MessageDecoderResult decodable(final IoSession session,
                                          final IoBuffer buffer) {
        MessageDecoderResult result;
        // Store the current cursor position in the buffer
        buffer.mark();
        // Need 2 bytes to check version and type
        if (buffer.remaining() < 4) {
            buffer.reset();
            result = MessageDecoderResult.NEED_DATA;
        } else {

            // Skip the version number
            // TODO: What to do with the version?
            buffer.get();
            final byte type = buffer.get();
            final int needRemaining = buffer.getUnsignedShort() - 4;
            // Reset the cursor so we don't modify the buffer data.
            buffer.reset();
            if (type == MessageType.UPDATE.value())
                if (buffer.remaining() >= needRemaining) {
                    result = MessageDecoderResult.OK;
                } else {
                    result = MessageDecoderResult.NEED_DATA;
                }
            else {
                result = MessageDecoderResult.NOT_OK;
            }
        }
        return result;
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * org.apache.mina.filter.codec.demux.MessageDecoder#decode(org.apache.mina
     * .core.session.IoSession, org.apache.mina.core.buffer.IoBuffer,
     * org.apache.mina.filter.codec.ProtocolDecoderOutput)
     */
    @Override
    public MessageDecoderResult decode(final IoSession session,
                                       final IoBuffer buffer, final ProtocolDecoderOutput out) {
    /*
     * Common message header stuff
     */
        final byte version = buffer.get();
        // Ignore type, since this is checked in decodable(IoSession, IoBuffer)
        buffer.get();
        // Ignore message length (not used)
        final int length = buffer.getUnsignedShort();
        final long requestId = buffer.getUnsignedInt();

        // Offset values
        final int optionsOffset = buffer.getUnsignedShort();
        final int payloadOffset = buffer.getUnsignedShort();

        // Origin address
        final AddressType addrType = AddressType.valueOf(buffer.getUnsignedShort());

        final int originAddrLength = buffer.getUnsignedShort();
        final byte[] originAddr = new byte[originAddrLength];
        buffer.get(originAddr);
        final NetworkAddress originAddress = new NetworkAddress(addrType,
                originAddr);

        LOG.debug("UpdateMessage:");
        LOG.debug("\tversion = {}", version);
        LOG.debug("\tlength = {}", length);
        LOG.debug("\trequestId = {}", requestId);
        LOG.debug("\toptionsOffset = {}", optionsOffset);
        LOG.debug("\tpayloadOffset = {}", payloadOffset);
        LOG.debug("\toriginAddress = {}", originAddress);

        final UpdateMessage msg = new UpdateMessage();
        msg.setVersion(version);
        msg.setRequestId(requestId);
        msg.setOriginAddress(originAddress);

        // update-specific stuff
        final GUID guid = new GUID();
        final byte[] guidBytes = new byte[GUID.SIZE_OF_GUID];
        buffer.get(guidBytes);
        guid.setBinaryForm(guidBytes);
        msg.setGuid(guid);
        LOG.debug("\tguid = {}", guid);

        final long numBindings = buffer.getUnsignedInt();
        LOG.debug("\tnumBindings = {}", numBindings);
        final NetworkAddress[] bindings = new NetworkAddress[(int) numBindings];

        for (int i = 0; i < numBindings; ++i) {
            final int addxType = buffer.getUnsignedShort();
            final int addxLength = buffer.getUnsignedShort();
            final byte[] addxBytes = new byte[addxLength];
            buffer.get(addxBytes);

            final NetworkAddress netAddr = new NetworkAddress(
                    AddressType.valueOf(addxType), addxBytes);
            bindings[i] = netAddr;
            LOG.debug("\tbindings[i] = {}", bindings[i]);
        }
        msg.setBindings(bindings);

        if(optionsOffset > 0){
            List<Option> options = RequestOptionsTranscoder.decode(buffer,length-optionsOffset);
            if(options != null && !options.isEmpty()){
                for(Option opt : options){
                    msg.addOption(opt);
                }
            }
        }

        // Send the decoded message to the next filter
        out.write(msg);

        //TODO: replace all print statements with a final print using toString for the message class

        // Everything OK!
        return MessageDecoderResult.OK;
    }

    @Override
    public void finishDecode(final IoSession arg0,
                             final ProtocolDecoderOutput arg1) {
        // Nothing to do
    }

}
