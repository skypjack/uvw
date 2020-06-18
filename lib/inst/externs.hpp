#ifndef UVW_EXTERNS_HPP
#define UVW_EXTERNS_HPP


namespace uvw {
    extern template UVW_EXTERN std::size_t details::type_factory<AsyncHandle>::event_type<AsyncEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<AsyncHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<AsyncHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<CheckHandle>::event_type<CheckEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<CheckHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<CheckHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<details::ConnectReq>::event_type<ConnectEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<details::ConnectReq>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<details::SendReq>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<details::SendReq>::event_type<SendEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<details::ShutdownReq>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<details::ShutdownReq>::event_type<ShutdownEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FakeHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FakeHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 11> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 14> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 15> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 16> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 1> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 27> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 2> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 3> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 4> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 5> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 8> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FileReq>::event_type<FsEvent<(details::UVFsType) 9> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsEventHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsEventHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsEventHandle>::event_type<FsEventEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsPollHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsPollHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsPollHandle>::event_type<FsPollEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 10> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 11> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 12> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 13> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 14> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 15> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 16> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 17> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 18> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 19> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 1> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 20> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 21> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 22> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 23> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 24> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 25> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 26> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 27> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 28> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 29> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 2> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 30> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 31> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 32> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 33> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 34> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 35> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 36> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 37> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 38> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 39> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 3> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 4> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 5> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 6> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 7> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 8> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<FsReq>::event_type<FsEvent<(details::UVFsType) 9> >() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<GetAddrInfoReq>::event_type<AddrInfoEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<GetAddrInfoReq>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<GetNameInfoReq>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<GetNameInfoReq>::event_type<NameInfoEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<IdleHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<IdleHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<IdleHandle>::event_type<IdleEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<Loop>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PipeHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PipeHandle>::event_type<ConnectEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PipeHandle>::event_type<DataEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PipeHandle>::event_type<EndEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PipeHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PipeHandle>::event_type<ListenEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PipeHandle>::event_type<ShutdownEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PipeHandle>::event_type<WriteEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PollHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PollHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PollHandle>::event_type<PollEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PrepareHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PrepareHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<PrepareHandle>::event_type<PrepareEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<ProcessHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<ProcessHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<ProcessHandle>::event_type<ExitEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<SignalHandle>::event_type<CheckEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<SignalHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<SignalHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<SignalHandle>::event_type<SignalEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TCPHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TCPHandle>::event_type<ConnectEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TCPHandle>::event_type<DataEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TCPHandle>::event_type<EndEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TCPHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TCPHandle>::event_type<ListenEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TCPHandle>::event_type<ShutdownEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TCPHandle>::event_type<WriteEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TimerHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TimerHandle>::event_type<TimerEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TTYHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TTYHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<TTYHandle>::event_type<WriteEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<UDPHandle>::event_type<CloseEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<UDPHandle>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<UDPHandle>::event_type<SendEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<UDPHandle>::event_type<UDPDataEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<WorkReq>::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<WorkReq>::event_type<WorkEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<WriteReq<std::default_delete<char []> > >::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<WriteReq<std::default_delete<char []> > >::event_type<WriteEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<WriteReq<void (*)(char*) > >::event_type<ErrorEvent>() noexcept;
    extern template UVW_EXTERN std::size_t details::type_factory<WriteReq<void (*)(char*) > >::event_type<WriteEvent>() noexcept;
}

#ifdef BUILD_TESTING_INSTANTIATIONS
#include "debug/externs.hpp"
#endif


#endif //UVW_EXTERNS_HPP
