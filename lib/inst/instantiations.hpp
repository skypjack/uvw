#ifndef UVW_INSTANTIATIONS_HPP
#define UVW_INSTANTIATIONS_HPP


namespace uvw {
    template std::size_t details::type_factory<AsyncHandle>::event_type<AsyncEvent>() noexcept;
    template std::size_t details::type_factory<AsyncHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<AsyncHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<CheckHandle>::event_type<CheckEvent>() noexcept;
    template std::size_t details::type_factory<CheckHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<CheckHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<details::ConnectReq>::event_type<ConnectEvent>() noexcept;
    template std::size_t details::type_factory<details::ConnectReq>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<details::SendReq>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<details::SendReq>::event_type<SendEvent>() noexcept;
    template std::size_t details::type_factory<details::ShutdownReq>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<details::ShutdownReq>::event_type<ShutdownEvent>() noexcept;
    template std::size_t details::type_factory<FakeHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<FakeHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 11> >() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 14> >() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 15> >() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 16> >() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 1> >() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 27> >() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 2> >() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 3> >() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 4> >() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 5> >() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 8> >() noexcept;
    template std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 9> >() noexcept;
    template std::size_t details::type_factory<FsEventHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<FsEventHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<FsEventHandle>::event_type<FsEventEvent>() noexcept;
    template std::size_t details::type_factory<FsPollHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<FsPollHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<FsPollHandle>::event_type<FsPollEvent>() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 10> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 11> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 12> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 13> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 14> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 15> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 16> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 17> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 18> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 19> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 1> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 20> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 21> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 22> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 23> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 24> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 25> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 26> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 27> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 28> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 29> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 2> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 30> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 31> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 32> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 33> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 34> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 35> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 36> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 37> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 38> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 39> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 3> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 4> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 5> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 6> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 7> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 8> >() noexcept;
    template std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 9> >() noexcept;
    template std::size_t details::type_factory<GetAddrInfoReq>::event_type<AddrInfoEvent>() noexcept;
    template std::size_t details::type_factory<GetAddrInfoReq>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<GetNameInfoReq>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<GetNameInfoReq>::event_type<NameInfoEvent>() noexcept;
    template std::size_t details::type_factory<IdleHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<IdleHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<IdleHandle>::event_type<IdleEvent>() noexcept;
    template std::size_t details::type_factory<Loop>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<PipeHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<PipeHandle>::event_type<ConnectEvent>() noexcept;
    template std::size_t details::type_factory<PipeHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<PollHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<PollHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<PollHandle>::event_type<PollEvent>() noexcept;
    template std::size_t details::type_factory<PrepareHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<PrepareHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<PrepareHandle>::event_type<PrepareEvent>() noexcept;
    template std::size_t details::type_factory<ProcessHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<ProcessHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<ProcessHandle>::event_type<ExitEvent>() noexcept;
    template std::size_t details::type_factory<SignalHandle>::event_type<CheckEvent>() noexcept;
    template std::size_t details::type_factory<SignalHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<SignalHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<SignalHandle>::event_type<SignalEvent>() noexcept;
    template std::size_t details::type_factory<TCPHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<TCPHandle>::event_type<ConnectEvent>() noexcept;
    template std::size_t details::type_factory<TCPHandle>::event_type<DataEvent>() noexcept;
    template std::size_t details::type_factory<TCPHandle>::event_type<EndEvent>() noexcept;
    template std::size_t details::type_factory<TCPHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<TCPHandle>::event_type<ListenEvent>() noexcept;
    template std::size_t details::type_factory<TCPHandle>::event_type<ShutdownEvent>() noexcept;
    template std::size_t details::type_factory<TCPHandle>::event_type<WriteEvent>() noexcept;
    template std::size_t details::type_factory<TimerHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<TimerHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<TimerHandle>::event_type<TimerEvent>() noexcept;
    template std::size_t details::type_factory<TTYHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<TTYHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<TTYHandle>::event_type<WriteEvent>() noexcept;
    template std::size_t details::type_factory<UDPHandle>::event_type<CloseEvent>() noexcept;
    template std::size_t details::type_factory<UDPHandle>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<UDPHandle>::event_type<SendEvent>() noexcept;
    template std::size_t details::type_factory<UDPHandle>::event_type<UDPDataEvent>() noexcept;
    template std::size_t details::type_factory<uvw::PipeHandle>::event_type<DataEvent>() noexcept;
    template std::size_t details::type_factory<uvw::PipeHandle>::event_type<EndEvent>() noexcept;
    template std::size_t details::type_factory<uvw::PipeHandle>::event_type<ListenEvent>() noexcept;
    template std::size_t details::type_factory<uvw::PipeHandle>::event_type<ShutdownEvent>() noexcept;
    template std::size_t details::type_factory<uvw::PipeHandle>::event_type<WriteEvent>() noexcept;
    template std::size_t details::type_factory<WorkReq>::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<WorkReq>::event_type<WorkEvent>() noexcept;
    template std::size_t details::type_factory<WriteReq<std::default_delete<char []> > >::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<WriteReq<std::default_delete<char []> > >::event_type<WriteEvent>() noexcept;
    template std::size_t details::type_factory<WriteReq<void (*)(char*) > >::event_type<ErrorEvent>() noexcept;
    template std::size_t details::type_factory<WriteReq<void (*)(char*) > >::event_type<WriteEvent>() noexcept;

}

#ifdef BUILD_TESTING_INSTANTIATIONS
#include "debug/instantiations.hpp"
#endif


#endif //UVW_INSTANTIATIONS_HPP
