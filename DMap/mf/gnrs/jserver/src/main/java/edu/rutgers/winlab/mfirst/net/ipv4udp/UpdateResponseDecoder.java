package edu.rutgers.winlab.mfirst.net.ipv4udp;

import edu.rutgers.winlab.mfirst.messages.UpdateResponseMessage;
import edu.rutgers.winlab.mfirst.messages.MessageType;
import edu.rutgers.winlab.mfirst.messages.ResponseCode;
import edu.rutgers.winlab.mfirst.messages.opt.Option;
import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import org.apache.mina.core.buffer.IoBuffer;
import org.apache.mina.core.session.IoSession;
import org.apache.mina.filter.codec.ProtocolDecoderOutput;
import org.apache.mina.filter.codec.demux.MessageDecoder;
import org.apache.mina.filter.codec.demux.MessageDecoderResult;

import java.util.List;

/**
 * Created by wontoniii on 4/24/15.
 */
public class UpdateResponseDecoder implements MessageDecoder {

    @Override
    public MessageDecoderResult decodable(final IoSession session,
                                          final IoBuffer buffer) {
        MessageDecoderResult result;
        // Store the current cursor position in the buffer
        buffer.mark();
        // Need 5 bytes to check request ID and type
        if (buffer.remaining() < 4) {
            result = MessageDecoderResult.NEED_DATA;
        } else {

            // Skip the version number
            // TODO: What happens with version number?
            buffer.get();
            final byte type = buffer.get();
            final int needRemaining = buffer.getUnsignedShort() - 4;
            // Reset the cursor so we don't modify the buffer data.
            buffer.reset();
            if (type == MessageType.UPDATE_RESPONSE.value()) {
                if (buffer.remaining() >= needRemaining) {
                    result = MessageDecoderResult.OK;
                } else {
                    result = MessageDecoderResult.NEED_DATA;
                }
            } else {
                result = MessageDecoderResult.NOT_OK;
            }
        }
        return result;

    }

    @Override
    public MessageDecoderResult decode(final IoSession session,
                                       final IoBuffer buffer, final ProtocolDecoderOutput out) {
    /*
     * Common message header stuff
     */
        // TODO: What to do with version?
        final byte version = buffer.get();
        final byte type = buffer.get();

        // Don't really care about message length
        int length = buffer.getUnsignedShort();
        final long requestId = buffer.getUnsignedInt();

        // Offsets
        final int optionsOffset = buffer.getUnsignedShort();
        final int payloadOffset = buffer.getUnsignedShort();

        final AddressType addrType = AddressType.valueOf(buffer.getUnsignedShort());

        final int originAddrLength = buffer.getUnsignedShort();
        final byte[] originAddr = new byte[originAddrLength];
        buffer.get(originAddr);
        final NetworkAddress originAddress = new NetworkAddress(addrType,
                originAddr);

        final UpdateResponseMessage msg = new UpdateResponseMessage();
        msg.setVersion(version);
        msg.setOriginAddress(originAddress);
        msg.setRequestId(requestId);

        // Response-specific stuff

        final int responseCode = buffer.getUnsignedShort();

        msg.setResponseCode(ResponseCode.valueOf(responseCode));

        // Remove unused padding
        buffer.getUnsignedShort();

        List<Option> options = RequestOptionsTranscoder.decode(buffer, length
                - optionsOffset);
        if (options != null) {
            for (Option opt : options) {
                msg.addOption(opt);
            }
        }

        // Write the decoded object to the next filter
        out.write(msg);

        return MessageDecoderResult.OK;
    }

    @Override
    public void finishDecode(final IoSession arg0,
                             final ProtocolDecoderOutput arg1) {
        // Nothing to do
    }

}